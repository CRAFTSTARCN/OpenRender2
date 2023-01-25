#pragma once
#include "OpenRenderRuntime/Modules/RenderData/RenderSwapDataResolver.h"

class TextureCreateDataResolver : public RenderSwapDataResolver
{

	uint32_t CheckLayer(const std::vector<std::vector<void*>>& TexData);

public:

	TextureCreateDataResolver(const RenderComponentsData& InData);

	void ResolveData(RenderSwapData* Data) override;
};