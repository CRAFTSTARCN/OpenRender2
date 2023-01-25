#include "AssetSystem.h"

#include <filesystem>
#include <stb_image.h>
#include <fstream>

#include "Importers/MaterialImporters.h"
#include "Importers/MeshImporters.h"
#include "Importers/TextureImporters.h"
#include "OpenRenderRuntime/Util/Logger.h"

std::string AssetSystem::LoadFile2Str(const std::string& FullPath)
{
	std::ifstream File;
	File.open(FullPath, std::ios::in);
	std::string Str;

	if(!File.is_open())
	{
		return Str;
	}

	File.seekg(0, std::ios::end);
	std::streamsize Len = File.tellg();
	File.seekg(0, std::ios::beg);

	char* Buffer = new char[Len + 1]{0};
	File.read(Buffer, Len);
	File.close();
	Str.assign(Buffer);
	delete[] Buffer;
	
	return  Str;
}

AssetSystem::AssetSystem(const AssetSystemCreateParam& Param)
	: SwapDataCenterPtr(nullptr),
	  Registry(1024)
{
	Config.BasePath = Param.BasePath;
	Config.ShaderPlatform = Param.ShaderPlatform;
	if(Param.TextureFlip)
	{
		stbi_set_flip_vertically_on_load(true);
	}
}

void AssetSystem::Init(RenderSwapDataCenter* InSwapDataCenter)
{
	SwapDataCenterPtr = InSwapDataCenter;
	AssetImportData CreateData {&Registry, SwapDataCenterPtr, &Config};
	Importers = {
		{"Texture2D", new Texture2DImporter{CreateData}},
		{"TextureCube", new TextureCubeImporter{CreateData}},
		{"MaterialBase", new MaterialBaseImporter{CreateData}},
		{"MaterialInstance", new MaterialInstanceImporter{CreateData}},
		{"StaticMeshPack", new MeshPackageImporter{CreateData}}
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
	Json AssetJson = Json::parse(LoadFile2Str(FullPath), Err);

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

