#include "OpenRenderRuntime/Modules/AssetSystem/Importers/TextureImporters.h"

#include <filesystem>

#include "OpenRenderRuntime/Modules/AssetSystem/AssetTypes/TextureAssets.h"
#include "OpenRenderRuntime/Modules/RenderData/TextureSwapData.h"
#include "OpenRenderRuntime/Util/Logger.h"
#include "stb_image.h"
#include "OpenRenderRuntime/Core/RenderResource/RenderResource.h"

TexturePixelFormat TextureImporter::AnalysisFormat(Json AssetJson)
{
	Json Format = AssetJson["Format"];
	const std::string& Str = Format.string_value();

	if(Str == std::string("SRGB"))
	{
		return TexturePixelFormat_RGBA8_SRGB;
	}
	else if(Str == std::string("HDR"))
	{
		return TexturePixelFormat_RGBA32_FLOAT;
	}
	else if(Str == std::string("Linear"))
	{
		return TexturePixelFormat_RGBA8;
	}
	else
	{
		LOG_WARN_FUNCTION("No format info in asset resource, default rgba will used");
	}
	return TexturePixelFormat_RGBA8;
	
}

TextureSamplerCreateStruct TextureImporter::AnalysisSample(Json AssetJson)
{
	TextureSamplerCreateStruct Sample {};
	static std::unordered_map<std::string, FilterType> FilterMap = {
		{std::string("Linear"), FilterType_Linear},
		{std::string("Nearest"), FilterType_Nearest},
		{std::string("Cubic"), FilterType_Cubic}};

	static std::unordered_map<std::string, TextureWrap> WrapMap = {
		{std::string("Repeat"), TextureWrap_Repeat},
		{std::string("MirrorRepeat"), TextureWrap_MirrorRepeat},
		{std::string("ClampToEdge"), TextureWrap_Clamp2Edge},
		{std::string("ClampToBorder"), TextureWrap_Clamp2Border}};

	//Filter
	auto FilterIter = FilterMap.find(AssetJson["MinFilter"].string_value());
	if(FilterIter == FilterMap.end())
	{
		LOG_WARN_FUNCTION("No min filter is asset, use linear");
		Sample.FilterMin = FilterType_Linear;
	}
	else
	{
		Sample.FilterMin = FilterIter->second;
	}

	FilterIter = FilterMap.find(AssetJson["MagFilter"].string_value());
	if(FilterIter == FilterMap.end())
	{
		LOG_WARN_FUNCTION("No mag filter is asset, use min filter value");
		Sample.FilterMag = Sample.FilterMin;
	}
	else
	{
		Sample.FilterMag = FilterIter->second;
	}

	//Mipmap filter
	const std::string& MipmapFilter = AssetJson["MipmapFilter"].string_value();
	if(MipmapFilter == std::string("Linear"))
	{
		Sample.MipmapFilter = MipmapFilterType_Linear;
	}
	else if (MipmapFilter == std::string("Nearest"))
	{
		Sample.MipmapFilter = MipmapFilterType_Nearest;
	}
	else
	{
		LOG_WARN_FUNCTION("No mipmap filter info in asset, use nearest");
		Sample.MipmapFilter = MipmapFilterType_Nearest;
	}

	//Wrap
	auto WrapIter = WrapMap.find(AssetJson["WrapU"].string_value());
	if(WrapIter == WrapMap.end())
	{
		LOG_WARN_FUNCTION("No u wrap info in asset, use repeat");
		Sample.WrapU = TextureWrap_Repeat;
	}
	else
	{
		Sample.WrapU = WrapIter->second;
	}

	WrapIter = WrapMap.find(AssetJson["WrapV"].string_value());
	if(WrapIter == WrapMap.end())
	{
		LOG_WARN_FUNCTION("No u wrap info in asset, use u");
		Sample.WrapV = Sample.WrapU;
	}
	else
	{
		Sample.WrapV = WrapIter->second;
	}
	Sample.WrapW = TextureWrap_Repeat;
	Sample.BorderColor[0] = (uint8_t)(AssetJson["BorderColor"]["R"].number_value() * 255.0);
	Sample.BorderColor[1] = (uint8_t)(AssetJson["BorderColor"]["G"].number_value() * 255.0);
	Sample.BorderColor[2] = (uint8_t)(AssetJson["BorderColor"]["B"].number_value() * 255.0);
	Sample.BorderColor[3] = (uint8_t)(AssetJson["BorderColor"]["A"].number_value() * 255.0);

	return Sample;
}


TextureImporter::TextureImporter(const AssetImportData& InData) : AssetImporter(InData)
{
}

TextureImporter::~TextureImporter() = default;

void TextureImporter::UnloadAsset(AssetObject* Asset)
{
	DYNAMIC_CAST_ASSET_CHECK(TextureAsset, Asset, TexAsset)
	
	if(SwapDataCenterPtr)
	{
		TextureDestroyData* TexDestroy = new TextureDestroyData;
		TexDestroy->Id = TexAsset->GetTexId();
		SwapDataCenterPtr->GetLogicSide().push_back(TexDestroy);
	}

	RegistryPtr->Unregister(TexAsset->GetId());
	delete TexAsset;
}

Texture2DImporter::Texture2DImporter(const AssetImportData& InData) : TextureImporter(InData)
{
}

Texture2DImporter::~Texture2DImporter()
{
}

size_t Texture2DImporter::LoadAsset(Json AssetJson, const std::string& RelPath)
{
	TexturePixelFormat Format = AnalysisFormat(AssetJson);
	const auto& Data = AssetJson["Data"].array_items();
	if(Data.empty())
	{
		LOG_ERROR_FUNCTION("No data in texture asset");
		return AssetRegistry::BAD_GASSET_ID;
	}

	std::vector<std::vector<void*>> TexData;
	uint32_t TexWidth = 0, TexHeight = 0;

	size_t Index = 0;
	for(const auto& Address : Data)
	{
		const std::string& AddressStr = Address.string_value();
		if(AddressStr.empty())
		{
			LOG_WARN_FUNCTION("Invalid value type for resource address, rest will not loaded");
			break;
		}

		std::string FullPath = (std::filesystem::path(ConfigPtr->BasePath) / AddressStr).generic_string();
		void* TexPtr = nullptr;
		int Width, Height, Channel;
		if(Format == TexturePixelFormat_RGBA32_FLOAT)
		{
			TexPtr = stbi_loadf(FullPath.c_str(), &Width, &Height, &Channel, STBI_rgb_alpha);
		}
		else
		{
			TexPtr = stbi_load(FullPath.c_str(), &Width, &Height, &Channel, STBI_rgb_alpha);
		}

		if(TexPtr == nullptr)
		{
			LOG_WARN_FUNCTION("Fail to load image resource from {0}, this and rest will ignored", FullPath.c_str());
			break;
		}

		if(Index == 0)
		{
			TexWidth = Width;
			TexHeight = Height;
		}
		else
		{
			if((uint32_t)Width * (uint32_t)Height < (TexWidth >> Index) * (TexHeight >> Index))
			{
				LOG_WARN_FUNCTION("Mipmap small, will cause memory read error");
				break;
			} 
		}

		TexData.push_back({TexPtr});
		++Index;
	}

	if(TexData.empty())
	{
		LOG_ERROR_FUNCTION("No valid data, texture create fail");
		return AssetRegistry::BAD_GASSET_ID;
	}

	ParamUsage Usage = AssetImporter::AnaParamUsage(AssetJson["UsageTime"].array_items());

	size_t TexId = RenderResource::TextureRegistry.GetNewId();
	Texture2DAsset* NewAsset = new Texture2DAsset(TexId);
	size_t AssetId = RegistryPtr->RegisterNew(NewAsset, RelPath);

	if(AssetId == AssetRegistry::BAD_GASSET_ID)
	{
		for(auto & TexPtr : TexData)
		{
			stbi_image_free(TexPtr.front());
		}
		delete NewAsset;
		return AssetId;
	}
	
	TextureCreateData *TexCreate = new TextureCreateData;
	TexCreate->CreateDataType = TextureType_2D;
	TexCreate->Format = Format;
	TexCreate->Width = TexWidth;
	TexCreate->Height = TexHeight;
	TexCreate->TexData = std::move(TexData);
	TexCreate->AllUsage = Usage;
	TexCreate->AutoMipmap = AssetJson["FilterMipmap"].bool_value();
	TexCreate->AutoMipmapLevelCount = std::max(AssetJson["AutoMipmapLevel"].int_value(), 1);
	TexCreate->SampleInfo = AnalysisSample(AssetJson);
	TexCreate->SampleAnisotropy = std::max(AssetJson["Anisotropy"].int_value(), 0);
	TexCreate->TextureId = TexId;
	SwapDataCenterPtr->GetLogicSide().push_back(TexCreate);

	return AssetId;
}

TextureCubeImporter::TextureCubeImporter(const AssetImportData& InData) : TextureImporter(InData)
{
}

TextureCubeImporter::~TextureCubeImporter()
{
}

size_t TextureCubeImporter::LoadAsset(Json AssetJson, const std::string& RelPath)
{
	static const char* FaceArr[6] = {"X+", "X-", "Y+", "Y-", "Z+", "Z-"};
	
	TexturePixelFormat Format = AnalysisFormat(AssetJson);
	const auto& Data = AssetJson["Data"].array_items();
	if(Data.empty())
	{
		LOG_ERROR_FUNCTION("Asset{0}: No data in texture asset", RelPath.c_str());
		return AssetRegistry::BAD_GASSET_ID;
	}

	std::vector<std::vector<void*>> TexData;
	uint32_t FaceWidth = 0, FaceHeight = 0;
	size_t Index = 0;

	for(const auto& Set : Data)
	{
		std::vector<void*> MipmapLevelData;
		for(size_t i = 0; i < 6; ++i)
		{
			const std::string& FacePath = Set[FaceArr[i]].string_value();
			if(FacePath.empty())
			{
				LOG_WARN_FUNCTION(
					"Asset {0}: No valid data type for face {1} image resource path, this and follow mipmap data ignored",
					RelPath.c_str(),
					FaceArr[i]);
				break;
			}

			std::string FullFacePath = (std::filesystem::path(ConfigPtr->BasePath) / FacePath).generic_string();

			int Width, Height, Channel;
			void* TexPtr = nullptr;
			if(Format == TexturePixelFormat_RGBA32_FLOAT)
			{
				TexPtr = stbi_loadf(FullFacePath.c_str(), &Width, &Height, &Channel, STBI_rgb_alpha);
			}
			else
			{
				TexPtr = stbi_load(FullFacePath.c_str(), &Width, &Height, &Channel, STBI_rgb_alpha);
			}

			if(!TexPtr)
			{
				LOG_WARN_FUNCTION("Asset{0}: No data in provided image resource path {1}", RelPath.c_str(), FacePath.c_str());
				break;
			}

			if(Index == 0)
			{
				if(i == 0)
				{
					FaceWidth = Width;
					FaceHeight = Height;
				}
				else if(FaceWidth != (uint32_t)Width || FaceHeight != (uint32_t)Height)
				{
					if((uint32_t)Width * (uint32_t)Height > FaceWidth * FaceHeight)
					{
						LOG_WARN_FUNCTION("Width and height not fit, but texel is larger, may cause tile");
					}
					else
					{
						LOG_WARN_FUNCTION("Face data small, will cause memory read error");
						break;
					}
				}
			}
			else
			{
				if((uint32_t)Width * (uint32_t)Height > (FaceWidth >> Index) * (FaceHeight >> Index))
				{
					LOG_WARN_FUNCTION("Width and height not fit, but texel is larger, may cause tile");
				}
				else if((uint32_t)Width * (uint32_t)Height < (FaceWidth >> Index) * (FaceHeight >> Index))
				{
					LOG_WARN_FUNCTION("Mipmap face data small, will cause memory read error");
					break;
				}
			}

			MipmapLevelData.push_back(TexPtr);
		}

		if(MipmapLevelData.size() != 6)
		{
			LOG_WARN_FUNCTION("Asset {}: No enough face, data, all follow mipmap deprecated", RelPath.c_str());
			break;
		}

		TexData.push_back(std::move(MipmapLevelData));
		Index++;
	}

	if(TexData.empty())
	{
		LOG_ERROR_FUNCTION("Asset{}: No valid data, texture create fail", RelPath);
		return AssetRegistry::BAD_GASSET_ID;
	}

	ParamUsage Usage = AssetImporter::AnaParamUsage(AssetJson["UsageTime"].array_items());
	size_t TexId = RenderResource::TextureRegistry.GetNewId();
	Texture2DAsset* NewAsset = new Texture2DAsset(TexId);
	size_t AssetId = RegistryPtr->RegisterNew(NewAsset, RelPath);

	if(AssetId == AssetRegistry::BAD_GASSET_ID)
	{
		for(auto& TexPtrs : TexData)
		{
			for(auto& TexPtr : TexPtrs)
			{
				stbi_image_free(TexPtr);
			}
		}
		delete NewAsset;
		return AssetId;
	}
	
	TextureCreateData *TexCreate = new TextureCreateData;
	TexCreate->CreateDataType = TextureType_Cube;
	TexCreate->Format = Format;
	TexCreate->Width = FaceWidth;
	TexCreate->Height = FaceHeight;
	TexCreate->TexData = std::move(TexData);
	TexCreate->AllUsage = Usage;
	TexCreate->AutoMipmap = AssetJson["AutoMipmap"].bool_value();
	TexCreate->AutoMipmapLevelCount = std::max(AssetJson["AutoMipmapLevel"].int_value(), 0);
	TexCreate->SampleInfo = AnalysisSample(AssetJson);
	TexCreate->SampleAnisotropy = std::max(AssetJson["Anisotropy"].int_value(), 0);
	TexCreate->TextureId = TexId;
	
	SwapDataCenterPtr->GetLogicSide().push_back(TexCreate);

	return AssetId;
	
}
