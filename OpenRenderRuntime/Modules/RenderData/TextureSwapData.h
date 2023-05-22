#pragma once
#include <vector>

#include "OpenRenderRuntime/Modules/RenderData/RenderSwapData.h"
#include "OpenRenderRuntime/Core/RHI/RHIEnums.h"
#include "OpenRenderRuntime/Core/RHI/RHITexture.h"


class TextureCreateData : public RenderSwapData
{
	
public:

	size_t TextureId = 0;
	uint32_t Width = 0;
	uint32_t Height = 0;
	ParamUsage AllUsage = ParamUsageBit_Geometry | ParamUsageBit_Fragment;
	RHIFormat Format = RHIFormat_RGB8_SRGB;
	TextureType CreateDataType = TextureType_2D;

	uint32_t SampleAnisotropy = 0;
	uint32_t AutoMipmapLevelCount = 1;
	bool AutoMipmap = false;
	
	std::vector<std::vector<void*>> TexData {};
	
	TextureSamplerCreateStruct SampleInfo {};
	
	DEFINE_SWAP_DATA_TYPE(TextureCreateData)
};

class TextureDestroyData : public RenderSwapData
{
	
public:

	size_t Id = 0;
	DEFINE_SWAP_DATA_TYPE(TextureDestroyData)
};

class DefaultTextureData : public RenderSwapData
{
	
public:

	size_t DefaultTextureId = 0;
	DEFINE_SWAP_DATA_TYPE(DefaultTextureData)
};

