#pragma once
#include "OpenRenderRuntime/Modules/AssetSystem/AssetObject.h"
#include "OpenRenderRuntime/Util/GUIDAllocator.h"

class AssetRegistry
{

	/*
	 * Asset id 0 is invalid
	 */
	GUIDAllocator<std::string> GAssetIDAllocator;
	std::vector<AssetObject*> AssetPool;
	
public:

	inline constexpr static size_t BAD_GASSET_ID = 0;
	
	AssetRegistry(size_t PrepareSpace);

	void Unregister(size_t AssetId);

	size_t RegisterNew(AssetObject* Object, const std::string& RelPath);

	size_t TryGetAssetId(const std::string& Path);

	AssetObject* GetObjectById(size_t Id);
	AssetObject* TryGetObjectByPath(const std::string& Path);

	void UnregisterAll(bool InternalDelete = true);
	std::vector<AssetObject*> GetAllAsset();
};
