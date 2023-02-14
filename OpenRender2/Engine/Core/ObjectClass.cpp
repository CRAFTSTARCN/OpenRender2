#include "OpenRender2/Engine/Core/ObjectClass.h"
#include "ClassRegister.h"

ObjectScript* ObjectClass::GetNewObject() const
{
	return ObjectFunction();
}

ObjectClass* ObjectClass::GetClassByName(const std::string& ClassName)
{
	return GClassHolder::Get().GetClass(ClassName);
}
