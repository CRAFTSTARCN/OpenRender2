#include "OpenRender2/Engine/Core/ObjectClass.h"

ObjectScript* ObjectClass::GetNewObject() const
{
	return ObjectFunction();
}
