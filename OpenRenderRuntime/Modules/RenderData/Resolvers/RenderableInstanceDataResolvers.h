#pragma once
#include "OpenRenderRuntime/Modules/RenderData/RenderSwapDataResolver.h"

class RenderableInstanceAddDataResolver : public RenderSwapDataResolver
{
	
public:

	RenderableInstanceAddDataResolver(const RenderComponentsData& InData);
	
	void ResolveData(RenderSwapData* Data) override;
};