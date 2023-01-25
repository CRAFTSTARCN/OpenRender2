#include "OpenRenderRuntime/Modules/RenderData/Resolvers/TextureDataResolvers.h"

#include <stb_image.h>

#include "OpenRenderRuntime/Core/RenderResource/RenderResource.h"
#include "OpenRenderRuntime/Modules/RenderData/TextureSwapData.h"
#include "OpenRenderRuntime/Util/Logger.h"

uint32_t TextureCreateDataResolver::CheckLayer(const std::vector<std::vector<void*>>& TexData)
{
	if(TexData.empty())
	{
		LOG_ERROR_FUNCTION("Empty textel data");
		return 0;
	}

	uint32_t LayerCount = (uint32_t)TexData[0].size();
	for(size_t i = 1; i < TexData.size(); ++i)
	{
		if(LayerCount < TexData[i].size())
		{
			LOG_ERROR_FUNCTION("Previous level: {}, mipmap {} level: {}, will use min", LayerCount, i, TexData[i].size());
		}
		else if(LayerCount > TexData[i].size())
		{
			LOG_ERROR_FUNCTION("Previous level: {}, mipmap {} level: {}, will use min", LayerCount, i, TexData[i].size());
			LayerCount  = (uint32_t)TexData[i].size();
		}
	}

	return LayerCount;
}

TextureCreateDataResolver::TextureCreateDataResolver(const RenderComponentsData& InData) : RenderSwapDataResolver(InData)
{
}

void TextureCreateDataResolver::ResolveData(RenderSwapData* Data)
{
	DYNAMIC_CAST_DATA_CHECK(TextureCreateData, Data, CreateData)

	auto FreeTextureCreationMemory = [CreateData]()
	{
		for(auto &TexMipmap : CreateData->TexData)
		{
			for(auto TexLayer : TexMipmap)
			{
				stbi_image_free(TexLayer);	
			}
		}

		delete CreateData;
	};

	if(ResourcePtr->Textures.find(CreateData->TextureId) != ResourcePtr->Textures.end())
	{
		FreeTextureCreationMemory();
		return;
	}

	uint32_t LayerCount = CheckLayer(CreateData->TexData);
	if(LayerCount == 0)
	{
		LOG_ERROR_FUNCTION("No texel layer");
		FreeTextureCreationMemory();
		return;
	}
	
	if(CreateData->CreateDataType == TextureType_Cube && LayerCount != 6)
	{
		LOG_ERROR_FUNCTION("Cube map need 6 layer(6 faces) but {} provided", LayerCount);
	}

	if(CreateData->CreateDataType == TextureType_2D && LayerCount != 1)
	{
		LOG_WARN_FUNCTION("{} layers of data provided, but tex provided, will use only first layer");
		LayerCount = 1;
	}

	RHITexture* Texture = nullptr;
	
	if(CreateData->AutoMipmap)
	{
		if(CreateData->TexData.size() != 1)
		{
			LOG_WARN_FUNCTION("Multi level of mipmaps provided for a auto mipmap texture");
		}
		Texture = RHIPtr->CreateTexture2DAutoMipmap(
			CreateData->Width,
			CreateData->Height,
			CreateData->Format,
			CreateData->TexData[0],
			CreateData->CreateDataType,
			CreateData->AllUsage,
			CreateData->AutoMipmapLevelCount,
			LayerCount,
			CreateData->SampleAnisotropy,
			CreateData->SampleInfo);
	}
	else
	{
		Texture = RHIPtr->CreateTexture2DManualMipmap(
			CreateData->Width,
			CreateData->Height,
			CreateData->Format,
			CreateData->TexData,
			CreateData->CreateDataType,
			CreateData->AllUsage,
			(uint32_t)CreateData->TexData.size(),
			CreateData->SampleAnisotropy,
			LayerCount,
			CreateData->SampleInfo);
	}

	if(!Texture)
	{
		LOG_ERROR_FUNCTION("Fail to create texture");
	}
	else
	{
		ResourcePtr->Textures.emplace(CreateData->TextureId, Texture);
	}
	
	FreeTextureCreationMemory();
}
