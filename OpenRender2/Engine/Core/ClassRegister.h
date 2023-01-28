#pragma once
#include <string>
#include <unordered_map>

class ObjectScript;
class ObjectClass;

#define DECLARE_OBJECT_CLASS(ClassName) \
private: \
static ClassRegister _##ClassName##ClassRegister;\
public: \
virtual ObjectClass* GetClass() const;\
private:

#define DEFINE_OBJECT_CLASS(ClassName) \
ClassRegister ClassName::_##ClassName##ClassRegister = ClassRegister(#ClassName, []()->ObjectScript* { return new ClassName{}; });\
\
ObjectClass* ClassName::GetClass() const{\
return ClassName::_##ClassName##ClassRegister.GetRegistered();\
}


class GClassHolder
{
	std::unordered_map<std::string, ObjectClass*> ClassTable;

	GClassHolder();

	
public:

	GClassHolder(const GClassHolder&) = delete;
	GClassHolder(GClassHolder&&) = delete;
	GClassHolder& operator=(const GClassHolder&) = delete;
	GClassHolder& operator=(const GClassHolder&&) = delete;

	void SetClass(std::string&& Name, ObjectClass* Class);
	ObjectClass* GetClass(const std::string& Name);
	
	static GClassHolder& Get();
};

class ClassRegister
{

	typedef ObjectScript* (*ObjectFuncType)();
	ObjectClass* RegisteredClass = nullptr;
	
public:

	ClassRegister(std::string&& Name, ObjectFuncType InstanceFunc);
	ObjectClass* GetRegistered();
};