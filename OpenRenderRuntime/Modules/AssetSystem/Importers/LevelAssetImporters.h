#pragma once
#include "OpenRenderRuntime/Modules/AssetSystem/AssetImporter.h"

class LevelAssetImporter : public AssetImporter
{
	
public:

	LevelAssetImporter(const AssetImportData& InData);
	~LevelAssetImporter();

	size_t LoadAsset(Json AssetJson, const std::string& RelPath) override;

	void UnloadAsset(AssetObject* Asset) override;
};
