#include "OpenRender2/Engine/Core/GObject.h"

#include "OpenRender2/Engine/Core/ObjectClass.h"
#include "OpenRender2/Engine/Core/TransformScript.h"
#include "OpenRenderRuntime/Util/Logger.h"

GObject::GObject() = default;

GObject::~GObject()
{
}

ObjectScript* GObject::CreateAndSerializeSingleScript(Json SingleScriptJson)
{
	const std::string& TypeName = SingleScriptJson["Type"].string_value();
	const std::string& ScriptName = SingleScriptJson["Name"].string_value();
	ObjectClass* ScriptObjectClass = ObjectClass::GetClassByName(TypeName);
	if(!ScriptObjectClass)
	{
		LOG_ERROR_FUNCTION(
			"Invalid class '{0}', for script {1}, object {2}",
			TypeName, ScriptName, Name);
		return nullptr;
	}

	ObjectScript* Script = ScriptObjectClass->GetNewObject();
	Script->Serialize(SingleScriptJson);
	Script->Register(this);
	return Script;
}

void GObject::Serialize(Json ObjectJson)
{
	Name = ObjectJson["Name"].string_value();
	auto && Scripts = ObjectJson["Scripts"].array_items();
	std::vector<ObjectScript*> SerializedScripts;
	SerializedScripts.reserve(Scripts.size());
	for(auto && S : Scripts)
	{
		ObjectScript* Script = CreateAndSerializeSingleScript(S);
		SerializedScripts.push_back(Script);
		TransformScript* Trans = dynamic_cast<TransformScript*>(Script);
		if(Trans)
		{
			int ParentIndex = Trans->GetInitialParentId();
			if(ParentIndex >= 0)
			{
				if(ParentIndex > (int)SerializedScripts.size())
				{
					LOG_ERROR_FUNCTION("Invalid parent index {} out of range, null will used", ParentIndex);
				}

				TransformScript* Par = dynamic_cast<TransformScript*>(SerializedScripts[ParentIndex]);;
				if(!Par)
				{
					LOG_ERROR_FUNCTION("Invalid index {}, indexed script is not transform script", ParentIndex);
				}
				else
				{
					Trans->SetParent(Par);
					Par->AddChild(Trans);
				}
			}
		}
	}

	for(auto Script : SerializedScripts)
	{
		if(Script)
		{
			Script->ConstructScript();
			OwnedScripts.insert(Script);
		}
	}
}

void GObject::Begin()
{
	for(auto Script : OwnedScripts)
	{
		Script->Begin();
	}
}

void GObject::Tick(float DeltaTime)
{
	TickArray.reserve(OwnedScripts.size());
	
	for(auto Script : OwnedScripts)
	{
		TickArray.push_back(Script);
	}

	for(auto TickableScript : TickArray)
	{
		TickableScript->TickScript(DeltaTime);
	}
}

void GObject::PostTick(float DeltaTime)
{
	for(auto TickableScript : TickArray)
	{
		TickableScript->PostTickScript(DeltaTime);
	}

	/*
	 * Avoid rapid memory allocation each frame
	 */
	TickArray.clear();
}

void GObject::Destroy()
{
	for(auto Script : OwnedScripts)
	{
		Script->Destroy();
	}

	std::unordered_set<ObjectScript*>().swap(OwnedScripts);
	Status = GObjectStatus_PendingKill;
}

void GObject::LevelFinishDestroy()
{
	if(Status != GObjectStatus_PendingKill)
	{
		LOG_ERROR_FUNCTION("Cannot finish destroy a living object");
	}
	delete this;
}

LLevel* GObject::CurrentLevel()
{
	return InLevel;
}

void GObject::SignToLevel(LLevel* Level)
{
	InLevel = Level;
}

void GObject::Activate()
{
	if(Status != GObjectStatus_Prepare)
	{
		LOG_ERROR_FUNCTION("Invalid status, object can only activated while prepare");
	}
	else
	{
		Status = GObjectStatus_Active;
		Begin();
	}
}

GObjectStatus GObject::GetStatus()
{
	return Status;
}

ObjectScript* GObject::AddNewScript(ObjectClass* ScriptClass, std::string&& InName)
{
	if(!ScriptClass)
	{
		LOG_ERROR_FUNCTION("Invalid class");
		return nullptr;
	}
	ObjectScript* NewObject = ScriptClass->GetNewObject();
	NewObject->Register(this);
	NewObject->RegisterName(std::move(InName));
	NewObject->ConstructScript();
	OwnedScripts.insert(NewObject);
	if(Status == GObjectStatus_Active)
	{
		NewObject->Begin();

	}

	return NewObject;
}

bool GObject::AddScript(ObjectScript* Script)
{
	if(!Script)
	{
		LOG_ERROR_FUNCTION("Invalid nullptr script add to {0}", Name);
	}
	auto && [Iter, Succ] =  OwnedScripts.insert(Script);
	if(Succ)
	{
		Script->Register(this);
	}
	return Succ;
}

void GObject::RemoveScript(ObjectScript* Script)
{
	OwnedScripts.erase(Script);
}

void GObject::DestroyAndRemove(ObjectScript* Script)
{
	auto Iter = OwnedScripts.find(Script);
	if(Iter == OwnedScripts.end())
	{
		LOG_ERROR_FUNCTION("{0}: Cnnot destroy script not belongs to this object", Name);
		return;
	}

	Script->Destroy();
	OwnedScripts.erase(Iter);
}

GObject* GObject::GetNewObject()
{
	return new GObject;
}
