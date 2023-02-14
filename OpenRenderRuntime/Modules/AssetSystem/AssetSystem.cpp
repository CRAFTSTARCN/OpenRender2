#include "AssetSystem.h"

#include <filesystem>
#include <stb_image.h>
#include <fstream>

#include "OpenRenderRuntime/Modules/AssetSystem/Importers/LevelAssetImporters.h"
#include "OpenRenderRuntime/Modules/AssetSystem/Importers/MaterialImporters.h"
#include "OpenRenderRuntime/Modules/AssetSystem/Importers/MeshImporters.h"
#include "OpenRenderRuntime/Modules/AssetSystem/Importers/TextureImporters.h"
#include "OpenRenderRuntime/Util/FileUtil.h"
#include "OpenRenderRuntime/Util/Logger.h"

AssetSystem::AssetSystem()
	: SwapDataCenterPtr(nullptr),
	  Registry(1024)
{
	
}

void AssetSystem::Init(const AssetSystemInitParam& InitParam)
{
	Config.BasePath = InitParam.BasePath;
	Config.ShaderPlatform = InitParam.ShaderPlatform;
	if(InitParam.TextureFlip)
	{
		stbi_set_flip_vertically_on_load(true);
	}
	
	SwapDataCenterPtr = InitParam.SwapDataCenter;
	AssetImportData CreateData {&Registry, SwapDataCenterPtr, &Config};
	Importers = {
		{"Texture2D", new Texture2DImporter{CreateData}},
		{"TextureCube", new TextureCubeImporter{CreateData}},
		{"MaterialBase", new MaterialBaseImporter{CreateData}},
		{"MaterialInstance", new MaterialInstanceImporter{CreateData}},
		{"StaticMeshPack", new MeshPackageImporter{CreateData}},
		{"Level", new LevelAssetImporter{CreateData}}
	};

	auto CascadeLoading = [this](const std::string& RelPath)
	{
		return this->OnCascadeLoading(RelPath);
	};
	
	Importers["MaterialInstance"]->SetCascadeLoadingFunction(CascadeLoading);
	Importers["StaticMeshPack"]->SetCascadeLoadingFunction(CascadeLoading);
}

size_t AssetSystem::Import(const std::string& RelPath)
{
	std::string Err;
	std::string FullPath = (std::filesystem::path(Config.BasePath) / RelPath).generic_string();
	Json AssetJson = Json::parse(FileUtil::LoadFile2Str(FullPath), Err);

	if(AssetJson.is_null())
	{
		LOG_ERROR_FUNCTION("Empty asset json or parse error {0}", RelPath.c_str());
		LOG_ERROR_FUNCTION("Json parse error: {0}", Err.c_str());
		return AssetRegistry::BAD_GASSET_ID;
	}

	const std::string& Type = AssetJson["AssetType"].string_value();
	
	auto Iter = Importers.find(Type);
	if(Iter == Importers.end())
	{
		LOG_ERROR_FUNCTION("No importer for type '{0}', {1}", Type.c_str(), RelPath.c_str());
		return AssetRegistry::BAD_GASSET_ID;
	}
	return Iter->second->LoadAsset(AssetJson, RelPath);
}

size_t AssetSystem::GetOrImport(const std::string& RelPath)
{
	size_t Id = Registry.TryGetAssetId(RelPath);
	if(Id == AssetRegistry::BAD_GASSET_ID)
	{
		return Import(RelPath);
	}
	return Id;
}

void AssetSystem::Unload(AssetObject*& AssetObj)
{
	auto Iter = Importers.find(AssetObj->GetAssetTypeName());
	if(Iter == Importers.end())
	{
		LOG_ERROR_FUNCTION("No importer found to uload asset, type {0}", AssetObj->GetAssetTypeName());
		delete AssetObj;
		return;
	}

	Iter->second->UnloadAsset(AssetObj);
	
	delete AssetObj;
}

size_t AssetSystem::OnCascadeLoading(const std::string& RelPath)
{
	return Import(RelPath);
}

AssetObject* AssetSystem::GetAssetById(size_t Id)
{
	return Registry.GetObjectById(Id);
}

const AssetObject* AssetSystem::GetAssetById(size_t Id) const
{
	return const_cast<AssetSystem*>(this)->Registry.GetObjectById(Id);
}

void AssetSystem::Terminate(bool SendDestroyToSwap)
{
	if(SendDestroyToSwap)
	{
		std::vector<AssetObject*> AllAsset = Registry.GetAllAsset();
		for(auto Asset : AllAsset)
		{
			Unload(Asset);
		}
	}

	Registry.UnregisterAll(false);
}

