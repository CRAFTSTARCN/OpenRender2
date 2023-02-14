#include "AssetRegistry.h"

#include "OpenRenderRuntime/Util/Logger.h"

AssetRegistry::AssetRegistry(size_t PrepareSpace) : GAssetIDAllocator(1) {
	
	AssetPool.reserve(PrepareSpace);
	AssetPool.resize(1);
}

void AssetRegistry::Unregister(size_t AssetId)
{
	if(AssetId >= AssetPool.size())
	{
		LOG_ERROR_FUNCTION("Invalid asset id {}", AssetId);
		return;
	}

	AssetPool[AssetId] = nullptr;
	GAssetIDAllocator.Deallocate(AssetId);
}

size_t AssetRegistry::RegisterNew(AssetObject* Object, const std::string& RelPath)
{
	if(!Object)
	{
		LOG_ERROR_FUNCTION("Null ptr asset can't registered");
		return BAD_GASSET_ID;
	}

	auto &&[Id, IsNew] = GAssetIDAllocator.GetOrAllocateNew(RelPath);
	if(!IsNew)
	{
		LOG_ERROR_FUNCTION("Duplicate path {}", RelPath.c_str());
		return BAD_GASSET_ID;
	}

	Object->OnFinishLoad(Id);
	if(Id == AssetPool.size())
	{
		AssetPool.push_back(Object);
	}
	else
	{
		AssetPool[Id] = Object;
	}

	return Id;
}

size_t AssetRegistry::TryGetAssetId(const std::string& Path)
{
	auto&& [Id, Exist] = GAssetIDAllocator.TryGetElement(Path);
	if(!Exist)
	{
		return BAD_GASSET_ID;
	}
	return Id;
}

AssetObject* AssetRegistry::GetObjectById(size_t Id)
{
	if(Id >= AssetPool.size() || !AssetPool[Id])
	{
		LOG_ERROR_FUNCTION("Invalid asset id {}", Id);
		return nullptr;
	}

	return AssetPool[Id];	
}

AssetObject* AssetRegistry::TryGetObjectByPath(const std::string& Path)
{
	size_t Id = TryGetAssetId(Path);
	if(Id == AssetRegistry::BAD_GASSET_ID)
	{
		return nullptr;
	}
	return GetObjectById(Id);
}

void AssetRegistry::UnregisterAll(bool InternalDelete)
{
	if(InternalDelete)
	{
		for(auto Asset : AssetPool)
		{
			delete Asset;
		}
	}

	GAssetIDAllocator.ResetAll();
}

std::vector<AssetObject*> AssetRegistry::GetAllAsset()
{
	std::vector<AssetObject*> Assets;
	Assets.reserve(AssetPool.size() >> 1);
	for(auto Asset : AssetPool)
	{
		if(Asset)
		{
			Assets.push_back(Asset);
		}
	}

	return  Assets;
}
