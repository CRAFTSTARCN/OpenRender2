#include "OpenRenderRuntime/Modules/AssetSystem/AssetObject.h"

AssetObject::AssetObject() : GAssetID(0xffffffffffffffff)
{
}

AssetObject::~AssetObject()
{
	
}

void AssetObject::OnFinishLoad(size_t Id)
{
	GAssetID = Id;
}

size_t AssetObject::GetId() const
{
	return GAssetID;
}

size_t AssetObject::GetUsageCount() const
{
	return UsageCount;
}

size_t AssetObject::IncreaseUsageCount()
{
	return (++UsageCount);
}

size_t AssetObject::DecreaseUsageCount()
{
	return (--UsageCount);
}
