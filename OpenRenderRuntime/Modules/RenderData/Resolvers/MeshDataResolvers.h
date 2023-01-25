#pragma once
#include "OpenRenderRuntime/Modules/RenderData/RenderSwapDataResolver.h"

class MeshDataCreateResolver : public RenderSwapDataResolver
{
	
public:
	
	MeshDataCreateResolver(const RenderComponentsData& InData);
	void ResolveData(RenderSwapData* Data) override;
};

class MeshDataDestroyResolver : public RenderSwapDataResolver
{

public:

	MeshDataDestroyResolver(const RenderComponentsData& InData);
	void ResolveData(RenderSwapData* Data) override;
};
