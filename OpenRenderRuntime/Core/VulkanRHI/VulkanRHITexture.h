#pragma once
#include <vulkan/vulkan_core.h>

#include "OpenRenderRuntime/Core/RHI/RHITexture.h"

class VulkanRHITexture : public RHITexture
{
	
public:

	VkSampler TextureSampler = VK_NULL_HANDLE;
};