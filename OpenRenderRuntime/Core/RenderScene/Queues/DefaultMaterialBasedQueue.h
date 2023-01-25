#pragma once
#include <unordered_map>

#include "OpenRenderRuntime/Core/RenderResource/RenderMaterialInstance.h"
#include "OpenRenderRuntime/Core/RenderResource/RenderMesh.h"
#include "OpenRenderRuntime/Core/RenderScene/RenderableInstance.h"

struct MeshSubQueue
{
	std::vector<size_t> MeshIndices {};
	RenderMesh* QueueMesh = nullptr;
	using IterType = std::vector<size_t>::iterator;
};

struct MaterialSubQueue
{
	std::unordered_map<size_t, MeshSubQueue> MeshTable {};
	RenderMaterialInstance* QueueMaterialInstance = nullptr;
	using IterType = std::unordered_map<size_t, MeshSubQueue>::iterator;
};

struct MaterialBaseSubQueue
{
	std::unordered_map<size_t, MaterialSubQueue> MaterialTable {};
	RenderMaterialBase* QueueMaterialBase = nullptr;
	using IterType = std::unordered_map<size_t, MaterialSubQueue>::iterator;
};

class DefaultMaterialBasedQueue 
{

	
	using InternalQueueIterType = std::unordered_map<size_t, MaterialBaseSubQueue>::iterator;

public:

	std::unordered_map<size_t, MaterialBaseSubQueue> InternalQueue;
	
	void ResetQueue();
	void InsertInstance(const RenderableInstance& Instance, size_t InstanceIndex);
};
