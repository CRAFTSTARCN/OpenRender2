#pragma once

#include "RenderSwapData.h"
#include "OpenRenderRuntime/Core/Basic/Transform.h"

class RenderableInstanceAdd : public RenderSwapData
{
	
public:

	size_t RenderableInstanceId = 0;
	size_t MeshId = 0;
	size_t MaterialInstanceId = 0;
	glm::mat4 ModelMatrix;

	bool IsBlocker = false;
	
	DEFINE_SWAP_DATA_TYPE(RenderableInstanceAdd)
};

class RenderableInstanceRemove : public RenderSwapData
{
	
public:

	size_t RenderableInstanceId = 0;
	DEFINE_SWAP_DATA_TYPE(RenderableInstanceRemove)
};