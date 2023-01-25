#pragma once
#include <functional>
#include <json11.hpp>
#include <string>

#include "OpenRenderRuntime/Core/RHI/RHIEnums.h"
#include "OpenRenderRuntime/Modules/AssetSystem/AssetRegistry.h"
#include "OpenRenderRuntime/Modules/AssetSystem/AssetSystemConfig.h"
#include "OpenRenderRuntime/Modules/RenderData/RenderSwapDataCenter.h"

#define DYNAMIC_CAST_ASSET_CHECK(AsserType, Var, NewName) \
	AsserType* NewName = dynamic_cast<AsserType*>(Var);\
	if(!NewName) {\
		LOG_ERROR_FUNCTION("Invalid cast asset type: {0}", #AsserType);\
		return;\
	}

using json11::Json;

struct AssetImportData
{
	AssetRegistry* RegistryPtr;
	RenderSwapDataCenter* SwapDataCenterPtr;
	const AssetSystemConfig* ConfigPtr;
};

class AssetImporter
{

protected:

	AssetRegistry* RegistryPtr;
	RenderSwapDataCenter* SwapDataCenterPtr;
	const AssetSystemConfig* ConfigPtr;
	std::function<size_t(const std::string&)> OnCascadeLoading;
	
public:

	AssetImporter(const AssetImportData& InData);

	void SetCascadeLoadingFunction(const std::function<size_t(const std::string&)>& Func);
	
	virtual ~AssetImporter();

	/*
	 * Loading an asset and return the GAssetID
	 */
	virtual size_t LoadAsset(Json AssetJson, const std::string& RelPath) = 0;

	virtual void UnloadAsset(AssetObject* Asset) = 0;

	ParamUsage AnaParamUsage(const std::vector<Json>& UsageJson);
};
