#pragma once
#include "OpenRenderRuntime/Modules/RenderData/RenderSwapDataResolver.h"

class CameraSetResolver : public RenderSwapDataResolver
{
	
public:

	CameraSetResolver(const RenderComponentsData& InData);
	void ResolveData(RenderSwapData* Data) override;
	
};

class CameraPositionLookResolver : public RenderSwapDataResolver
{
	
public:

	CameraPositionLookResolver(const RenderComponentsData& InData);
	void ResolveData(RenderSwapData* Data) override;
};

class DirectionalLightResolver : public RenderSwapDataResolver
{
	
public:
	
	DirectionalLightResolver(const RenderComponentsData& InData);
	void ResolveData(RenderSwapData* Data) override;
};

class EnvironmentDataResolver : public RenderSwapDataResolver
{
	
public:

	EnvironmentDataResolver(const RenderComponentsData& InData);
	void ResolveData(RenderSwapData* Data) override;
};

