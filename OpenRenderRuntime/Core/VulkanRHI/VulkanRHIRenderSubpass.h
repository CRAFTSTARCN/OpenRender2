#pragma once
#include <cstdint>

#include "OpenRenderRuntime/Core/RHI/RHIRenderSubpass.h"

class VulkanRHIRenderSubpass : public RHIRenderSubpass
{
	
public:
	uint32_t SubpassIndex = 0;
};
