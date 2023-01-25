#pragma once

#include <string>

#include "OpenRenderRuntime/Util/GUIDAllocator.h"

#define STR_CLASS(ClassName) #ClassName 

#define DEFINE_SWAP_DATA_TYPE(ClassName) \
	std::string GetDataType() const override {\
		return STR_CLASS(ClassName);\
	}\
\
	size_t GetTypeId() const override {\
		static auto Id = RenderSwapData::IDAllocator.GetAllocateNew(#ClassName);\
		return Id;\
	}

class RenderSwapData
{
protected:
	
	inline static GUIDAllocator<std::string> IDAllocator{0};
	
public:

	virtual ~RenderSwapData() = default;
	inline virtual std::string GetDataType() const = 0;
	inline virtual size_t GetTypeId() const = 0;
};
