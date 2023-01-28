#pragma once
#include <string>

class ObjectScript;

class ObjectClass
{

	typedef ObjectScript* (*ObjectFuncType)();
	
	ObjectFuncType ObjectFunction = nullptr;
	std::string ClassName;


	friend class ClassRegister;
	
public:

	ObjectScript* GetNewObject() const;
		
};
