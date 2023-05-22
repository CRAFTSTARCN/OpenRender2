#pragma once
#include "OpenRenderRuntime/Core/RHI/RHITexture.h"
#include "OpenRenderRuntime/Modules/AssetSystem/AssetImporter.h"

class TextureImporter : public AssetImporter
{

protected:

	RHIFormat AnalysisFormat(Json AssetJson);
	TextureSamplerCreateStruct AnalysisSample(Json AssetJson);
	
public:
	
	TextureImporter(const AssetImportData& InData);
	virtual ~TextureImporter() override;

	void UnloadAsset(AssetObject* Asset) override;
};

class Texture2DImporter final : public TextureImporter
{
	
public:

	Texture2DImporter(const AssetImportData& InData);
	~Texture2DImporter() override;

	size_t LoadAsset(Json AssetJson, const std::string& RelPath) override;
};

class TextureCubeImporter final : public  TextureImporter
{
	
public:
	TextureCubeImporter(const AssetImportData& InData);
	~TextureCubeImporter() override;
	size_t LoadAsset(Json AssetJson, const std::string& RelPath) override;
};