#include "OpenRender2/Engine/Core/GObject.h"

GObject::GObject() = default;

GObject::~GObject()
{
}

void GObject::Serialize(Json ObjectJson)
{
	//TODO: Serialize Object
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
	for(auto Script : OwnedScripts)
	{
		Script->TickScript(DeltaTime);
	}
}

void GObject::Destroy()
{
	//TODO: Destruction
}

GObject* GObject::GetNewObject()
{
	return new GObject;
}
