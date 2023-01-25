#pragma once
#include <string>

#include "OpenRenderRuntime/Modules/AssetSystem/AssetImporter.h"
#include "OpenRenderRuntime/Modules/AssetSystem/AssetRegistry.h"
#include "OpenRenderRuntime/Modules/RenderData/RenderSwapDataCenter.h"
#include "OpenRenderRuntime/Modules/AssetSystem/AssetSystemConfig.h"

struct AssetSystemCreateParam
{
	std::string BasePath;
	std::string ShaderPlatform;

	bool TextureFlip = false;
};


class AssetSystem
{

	AssetSystemConfig Config;

	RenderSwapDataCenter* SwapDataCenterPtr;

	AssetRegistry Registry;

	std::unordered_map<std::string, AssetImporter*> Importers;

	std::string LoadFile2Str(const std::string& FullPath);

public:
	
	AssetSystem(const AssetSystemCreateParam& Param);

	void Init(RenderSwapDataCenter* InSwapDataCenter);

	size_t Import(const std::string& RelPath);

	size_t GetOrImport(const std::string& RelPath);
	
	/*
	 * Asset object will unload, all pointer will be a invalid one
	 */
	void Unload(AssetObject*& AssetObj);

	size_t OnCascadeLoading(const std::string& RelPath);

	AssetObject* GetAssetById(size_t Id);

	const AssetObject* GetAssetById(size_t Id) const;

	//void ClearAll();
};
