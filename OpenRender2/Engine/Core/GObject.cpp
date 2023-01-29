#include "OpenRender2/Engine/Core/GObject.h"

GObject::GObject() = default;

GObject::~GObject()
{
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

GObject* GObject::GetNewObject()
{
	return new GObject;
}
