#pragma once
#include "OpenRenderRuntime/Core/RenderResource/RenderMaterialBase.h"

class RenderMaterialInstance
{
	
public:
	
	inline static AutoIncreaseIdAllocator Registry {0};
	constexpr static size_t BAD_MATERIAL_INSTANCE_ID = SIZE_MAX;

	size_t Id = 0;
	RenderMaterialBase* Base = nullptr;
	RHIDescriptorSet* MaterialDescriptorSet = nullptr;
	RHIBuffer* MaterialParamUniform = nullptr;
	
};
