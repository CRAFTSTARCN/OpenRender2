#include "OpenRender2/Engine/Core/ObjectScript.h"
#include "OpenRender2/Engine/Core/ObjectClass.h"

ObjectScript::ObjectScript() : Owner(nullptr)
{
	
}

ObjectScript::~ObjectScript() = default;

void ObjectScript::Serialize(Json ScriptJson)
{
	
}

void ObjectScript::Register(GObject* InOwner)
{
	Owner = InOwner;
}

void ObjectScript::Begin()
{
}

void ObjectScript::TickScript(float DeltaTime)
{
	
}

void ObjectScript::Destroy()
{
	OnDestroy();
	delete this;
}

void ObjectScript::OnDestroy()
{
}

DEFINE_OBJECT_CLASS(ObjectScript)

