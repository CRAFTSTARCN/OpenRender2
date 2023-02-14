#pragma once
#include <string>

#include "OpenRenderRuntime/Util/GUIDAllocator.h"

#define DEFINE_ASSET_TYPE(AssetTypeName)\
	std::string GetAssetTypeName() const override {\
		return AssetTypeName;\
	}\
\
	size_t GetAssetTypeId() const override {\
		static auto Id = AssetTypeIdAllocator.GetAllocateNew(AssetTypeName);\
		return Id;\
	}
	

class AssetObject
{
protected:

	inline static GUIDAllocator<std::string> AssetTypeIdAllocator{0};
	
	size_t GAssetID = 0;

	/*
	 * Internal reference maintained by asset system
	 * (for example, when a new material instance loaded, and declare a usage of a texture,
	 * the usage count of that texture will increase by one)
	 * External reference should maintained by user
	 */
	size_t UsageCount = 0;

public:

	AssetObject();

	virtual ~AssetObject();

	virtual void OnFinishLoad(size_t Id);
	
	size_t GetId() const;

	virtual std::string GetAssetTypeName() const = 0;
	virtual size_t GetAssetTypeId() const = 0;

	size_t GetUsageCount() const;
	size_t IncreaseUsageCount();
	size_t DecreaseUsageCount();
};
