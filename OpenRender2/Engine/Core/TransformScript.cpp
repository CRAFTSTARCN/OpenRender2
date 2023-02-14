#include "OpenRender2/Engine/Core/TransformScript.h"
#include "OpenRender2/Engine/Core/ObjectClass.h"
#include "OpenRenderRuntime/Core/Basic/RenderMath.h"
#include "OpenRenderRuntime/Util/VecSerializer.h"

void TransformScript::Serialize(Json ScriptJson)
{
	ObjectScript::Serialize(ScriptJson);

	if(ScriptJson["Parent"].is_number())
	{
		InitialParentIndex = ScriptJson["Parent"].int_value();
	}

	auto TransFormJson = ScriptJson["Transform"];
	if(TransFormJson.is_null())
	{
		RelativeTransform = ORMath::GetIdentityTransform();
		return;
	}

	RelativeTransform.Translate = VecSerializer::SerializeVec3(TransFormJson["Translate"]);
	RelativeTransform.Rotation = glm::quat(glm::radians(VecSerializer::SerializeVec3(TransFormJson["Rotation"])));
	RelativeTransform.Scale = VecSerializer::SerializeVec3(TransFormJson["Scale"]);
	
	CalculateTransformRel();
}

int TransformScript::GetInitialParentId()
{
	return InitialParentIndex;
}

void TransformScript::SetParent(TransformScript* InParent)
{
	Parent = InParent;
}

TransformScript* TransformScript::GetParent()
{
	return Parent;
}

bool TransformScript::AddChild(TransformScript* Child)
{
	auto && [Iter, Succ] = ChildScripts.insert(Child);
	return Succ;
}

void TransformScript::RemoveChild(TransformScript* Child)
{
	ChildScripts.erase(Child);
}

void TransformScript::CalculateTransformRel()
{
	TransformMatCache = ORMath::Transform2Matrix(RelativeTransform);
	Dirty = false;
}

const glm::mat4& TransformScript::GetTransformMatRel()
{
	if(Dirty)
	{
		CalculateTransformRel();
	}

	return TransformMatCache;
}

glm::mat4 TransformScript::GetTransformMatAbs()
{
	glm::mat4 Res = GetTransformMatRel();
	TransformScript* Par = Parent;

	/*
	 * Use circle instead of recursion
	 */
	while(Par)
	{
		Res = Par->GetTransformMatRel() * Res;
		Par = Par->GetParent();
	}
	
	return Res;
}

DEFINE_OBJECT_CLASS(TransformScript)
