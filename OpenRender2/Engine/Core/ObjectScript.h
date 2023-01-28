#pragma once
#include <json11.hpp>
#include "OpenRender2/Engine/Core/ClassRegister.h"

using Json = json11::Json;

class ObjectClass;

class ObjectScript
{

	DECLARE_OBJECT_CLASS(ObjectScript)
	
public:


	ObjectScript();
	virtual ~ObjectScript();
	
	virtual void Init(Json ScriptJson);

	virtual void TickScript(float DeltaTime);
	
};