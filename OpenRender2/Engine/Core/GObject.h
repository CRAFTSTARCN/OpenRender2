#pragma once
#include <unordered_set>
#include <vector>

#include "OpenRender2/Engine/Core/ObjectScript.h"
#include "OpenRenderRuntime/Util/AutoIncreaseIdAllocator.h"

class GObject final
{

	inline static AutoIncreaseIdAllocator GObjectIdAllocator{0};

	std::string Name;
	
	std::unordered_set<ObjectScript*> OwnedScripts;

	GObject();

	~GObject();
	
public:

	GObject(const GObject&) = delete;
	GObject(GObject&&) = delete;
	GObject& operator=(const GObject&) = delete;
	GObject& operator=(GObject&&) = delete;
	
	void Serialize(Json ObjectJson);

	void Begin();

	void Tick(float DeltaTime);

	void Destroy();

	/*
	 * The data in script will remain as default
	 */
	void AddNewScript(ObjectClass* ScriptClass, std::string&& Name = std::string(""));

	void AddScript(ObjectScript* Script);
	
	static GObject* GetNewObject();
};
