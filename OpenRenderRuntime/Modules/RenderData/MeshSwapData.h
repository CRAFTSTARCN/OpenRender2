#pragma once
#include <vector>

#include "OpenRenderRuntime/Core/Basic/Meshlet.h"
#include "OpenRenderRuntime/Core/Basic/BoundingVolumes.h"
#include "OpenRenderRuntime/Core/Basic/VertexData.h"
#include "OpenRenderRuntime/Modules/RenderData/RenderSwapData.h"

class MeshCreateData : public RenderSwapData
{
	
public:

	size_t MeshId = 0;
	std::vector<VertexData> VertexData {};
	MeshletPackage MeshletPackage {};
	AABB MeshOriginalAABB {};
	
	DEFINE_SWAP_DATA_TYPE(MeshCreateData)
};

class MeshDestroyData : public RenderSwapData
{

public:

	size_t MeshId = 0;
	DEFINE_SWAP_DATA_TYPE(MeshDestroyData)
};