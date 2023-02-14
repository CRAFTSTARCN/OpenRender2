#pragma once

#include <unordered_set>

#include "OpenRender2/Engine/Core/ObjectScript.h"
#include "OpenRenderRuntime/Core/Basic/Transform.h"

class TransformScript : public ObjectScript
{

	DECLARE_OBJECT_CLASS(TransformScript)
	
protected:

	int InitialParentIndex = -1;
	TransformScript* Parent = nullptr;

	Transform RelativeTransform;
	std::unordered_set<TransformScript*> ChildScripts;

	bool Dirty = false;
	glm::mat4 TransformMatCache {1};
	
public:


	void Serialize(Json ScriptJson) override;
	
	//Only for serialize time
	int GetInitialParentId();

	void SetParent(TransformScript* InParent);

	TransformScript* GetParent();

	bool AddChild(TransformScript* Child);

	void RemoveChild(TransformScript* Child);

	void CalculateTransformRel();

	const glm::mat4& GetTransformMatRel();

	glm::mat4 GetTransformMatAbs();
};
