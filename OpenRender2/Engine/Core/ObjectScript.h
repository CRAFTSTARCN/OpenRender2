#pragma once
#include <json11.hpp>
#include "OpenRender2/Engine/Core/ClassRegister.h"

using json11::Json;

class ObjectClass;
class GObject;

class ObjectScript
{

	DECLARE_OBJECT_CLASS(ObjectScript)

protected:
	std::string Name;

	GObject* Owner;
	
	virtual ~ObjectScript();

	ObjectScript();
	
public:

	ObjectScript(const ObjectScript&) = delete;
	ObjectScript(ObjectScript&&) = delete;

	ObjectScript& operator=(const ObjectScript&) = delete;
	ObjectScript& operator=(ObjectScript&&) = delete;
	
	virtual void Serialize(Json ScriptJson);

	virtual void Register(GObject* InOwner);

	virtual void RegisterName(std::string&& InName);

	virtual void ConstructScript();

	virtual void Begin();

	virtual void TickScript(float DeltaTime);

	virtual void PostTickScript(float DeltaTime);
	
	void Destroy();

	virtual void OnDestroy();
};