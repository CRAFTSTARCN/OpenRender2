#pragma once
#include <unordered_set>
#include <vector>

#include "OpenRender2/Engine/Core/ObjectScript.h"
#include "OpenRenderRuntime/Util/AutoIncreaseIdAllocator.h"

class LLevel;

enum GObjectStatus
{
	GObjectStatus_Prepare,
	GObjectStatus_Active,
	GObjectStatus_PendingKill
};

class GObject final
{

	inline static AutoIncreaseIdAllocator GObjectIdAllocator{0};

	std::string Name;
	
	std::unordered_set<ObjectScript*> OwnedScripts;

	std::vector<ObjectScript*> TickArray;

	LLevel* InLevel = nullptr;

	GObjectStatus Status = GObjectStatus_Prepare;

	GObject();

	~GObject();

	ObjectScript* CreateAndSerializeSingleScript(Json SingleScriptJson);

	void Begin();
	
public:

	GObject(const GObject&) = delete;
	GObject(GObject&&) = delete;
	GObject& operator=(const GObject&) = delete;
	GObject& operator=(GObject&&) = delete;
	
	void Serialize(Json ObjectJson);
	
	void Tick(float DeltaTime);

	void PostTick(float DeltaTime);

	void Destroy();

	/*
	 * Should only call by level
	 */
	void LevelFinishDestroy();
	
	LLevel* CurrentLevel();

	void SignToLevel(LLevel* Level);

	void Activate();
	
	GObjectStatus GetStatus();
	
	/*
	 * The data in script will remain as default
	 */
	ObjectScript* AddNewScript(ObjectClass* ScriptClass, std::string&& InName = std::string(""));

	bool AddScript(ObjectScript* Script);

	void RemoveScript(ObjectScript* Script);

	/*
	 * Convenience function, equals to Script->Destroy(); RemoveScript(Script);
	 * But ensure the script belongs to this object
	 */
	void DestroyAndRemove(ObjectScript* Script);
	
	static GObject* GetNewObject();
};
