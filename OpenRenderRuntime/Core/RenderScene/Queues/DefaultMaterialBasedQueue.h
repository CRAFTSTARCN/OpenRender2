#pragma once
#include <unordered_map>

#include "OpenRenderRuntime/Core/RenderResource/RenderMaterialInstance.h"
#include "OpenRenderRuntime/Core/RenderResource/RenderMesh.h"
#include "OpenRenderRuntime/Core/RenderScene/RenderableInstance.h"
#include "OpenRenderRuntime/Core/RenderScene/RenderQueue.h"

class RenderScene;

struct MeshSubQueue
{
	std::vector<size_t> InstanceIndices {};
};

struct MaterialSubQueue
{
	std::unordered_map<RenderMesh*, MeshSubQueue> MeshTable {};
};

struct MaterialBaseSubQueue
{	
	std::unordered_map<RenderMaterialInstance*, MaterialSubQueue> MaterialTable {};
};

class DefaultMaterialBasedQueue : public RenderQueue
{

public:

	uint32_t SceneBit = 0;
	
	std::unordered_map<RenderMaterialBase*, MaterialBaseSubQueue> InternalQueue;
	
	void Form() override;
	
	void ResetQueue() override;
	void InsertInstance(const RenderableInstance& Instance, size_t InstanceIndex);
};
