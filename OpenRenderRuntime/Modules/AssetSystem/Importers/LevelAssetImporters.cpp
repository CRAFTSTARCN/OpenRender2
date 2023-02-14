#include "OpenRenderRuntime/Modules/AssetSystem/Importers/LevelAssetImporters.h"

#include "OpenRenderRuntime/Modules/AssetSystem/AssetTypes/LevelAssets.h"
#include "OpenRenderRuntime/Util/VecSerializer.h"

LevelAssetImporter::LevelAssetImporter(const AssetImportData& InData) : AssetImporter(InData)
{
}

LevelAssetImporter::~LevelAssetImporter() = default;

size_t LevelAssetImporter::LoadAsset(Json AssetJson, const std::string& RelPath)
{
	LevelAsset* NewAsset = new LevelAsset;
	
	NewAsset->ObjectJson = AssetJson["Objects"].array_items();

	auto SkyboxJson = AssetJson["Skybox"];
	NewAsset->SkyboxData.Skybox = SkyboxJson["Skybox"].string_value();
	NewAsset->SkyboxData.Radiance = SkyboxJson["Radiance"].string_value();
	NewAsset->SkyboxData.Irradiance = SkyboxJson["Irradiance"].string_value();

	auto CamJson = AssetJson["Camera"];
	NewAsset->InitCameraData.Position = VecSerializer::SerializeVec3(CamJson["Position"]);
	NewAsset->InitCameraData.Rotation = VecSerializer::SerializeVec3(CamJson["Rotation"]);
	NewAsset->InitCameraData.Near = (float)CamJson["Near"].number_value();
	NewAsset->InitCameraData.Far = CamJson["Far"].is_number() ?  (float)CamJson["Far"].number_value() : 10000.0f;
	NewAsset->InitCameraData.FOVV = CamJson["FOVV"].is_number() ?  (float)CamJson["FOVV"].number_value() : 75.0f;
	NewAsset->InitCameraData.CameraMoveSpeed = CamJson["CameraMoveSpeed"].is_number() ?  (float)CamJson["CameraMoveSpeed"].number_value() : 70.0f;
	NewAsset->InitCameraData.CameraLookSpeed = CamJson["CameraLookSpeed"].is_number() ?  (float)CamJson["CameraLookSpeed"].number_value() : 0.1f;

	auto LightJson = AssetJson["DirectionalLight"];
	NewAsset->LightData.Color = VecSerializer::SerializeColor(LightJson["Color"]);
	NewAsset->LightData.Direction = glm::normalize(VecSerializer::SerializeVec3(LightJson["Direction"]));
	
	size_t AssetId = RegistryPtr->RegisterNew(NewAsset, RelPath);
	if(AssetId == AssetRegistry::BAD_GASSET_ID)
	{
		LOG_ERROR_FUNCTION("Asset path may duplicate");
		delete NewAsset;
	}

	return AssetId;
}

void LevelAssetImporter::UnloadAsset(AssetObject* Asset)
{
	DYNAMIC_CAST_ASSET_CHECK(LevelAsset, Asset, Level)
	RegistryPtr->Unregister(Level->GetId());
	delete Level;
}
