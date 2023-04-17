#pragma once
#include <unordered_map>

#include "OpenRenderRuntime/Core/RenderResource/RenderMaterialInstance.h"
#include "OpenRenderRuntime/Core/RenderResource/RenderMesh.h"
#include "OpenRenderRuntime/Core/RenderScene/RenderableInstance.h"

class RenderScene;

struct MeshSubQueue
{
	std::mutex MeshMutex;
	std::vector<size_t> InstanceIndices {};
};

struct MaterialSubQueue
{
	std::mutex MaterialInstanceMutex;
	std::unordered_map<RenderMesh*, MeshSubQueue*> MeshTable {};
};

struct MaterialBaseSubQueue
{	
	std::mutex MaterialBaseMutex;
	std::unordered_map<RenderMaterialInstance*, MaterialSubQueue*> MaterialTable {};
};

class DefaultMaterialBasedQueue 
{

	
	RenderScene* Scene = nullptr;
	std::mutex GlobalMutex;
	
public:

	std::unordered_map<RenderMaterialBase*, MaterialBaseSubQueue*> InternalQueue;

	void SetScene(RenderScene* InScene);

	void Form();
	
	void ResetQueue();
	void InsertInstance(const RenderableInstance& Instance, size_t InstanceIndex);
};
