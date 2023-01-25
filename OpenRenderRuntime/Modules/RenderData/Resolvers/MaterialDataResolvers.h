#pragma once
#include "OpenRenderRuntime/Modules/RenderData/RenderSwapDataResolver.h"

class MaterialBaseDataCreateResolver : public RenderSwapDataResolver
{
	
public:

	MaterialBaseDataCreateResolver(const RenderComponentsData& InData);

	void ResolveData(RenderSwapData* Data) override;
};

class MaterialInstanceDataCreateResolver : public RenderSwapDataResolver
{
	
public:

	MaterialInstanceDataCreateResolver(const RenderComponentsData& InData);

	void ResolveData(RenderSwapData* Data) override;
};