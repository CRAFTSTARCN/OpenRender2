#include "OpenRender2/Engine/Core/ClassRegister.h"

#include "ObjectClass.h"
#include "OpenRenderRuntime/Util/Logger.h"

GClassHolder::GClassHolder() = default;

void GClassHolder::SetClass(std::string&& Name, ObjectClass* Class)
{
	auto && [Iter, Succ] = ClassTable.try_emplace(std::move(Name), Class);
	if(!Succ)
	{
		LOG_ERROR_FUNCTION("Duplicate registry on class {0}, fatal error", Name);
		assert(false);
	}
}

ObjectClass* GClassHolder::GetClass(const std::string& Name)
{
	auto Iter = ClassTable.find(Name);
	if(Iter == ClassTable.end())
	{
		return nullptr;
	}

	return Iter->second;
}

GClassHolder& GClassHolder::Get()
{
	static GClassHolder GlobalHolder;
	return GlobalHolder;
}

ClassRegister::ClassRegister(std::string&& Name, ObjectFuncType InstanceFunc)
{
	ObjectClass* Class = new ObjectClass;
	Class->ClassName = Name;
	Class->ObjectFunction = InstanceFunc;
	RegisteredClass = Class;
	GClassHolder::Get().SetClass(std::move(Name), Class);
}

ObjectClass* ClassRegister::GetRegistered()
{
	return RegisteredClass;
}
