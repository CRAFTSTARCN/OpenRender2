#pragma once
#include <vulkan/vulkan_core.h>

#include "OpenRenderRuntime/Core/RHI/RHITexture.h"

class VulkanRHITexture : public RHITexture
{
	
public:
	
	VkImage Image = VK_NULL_HANDLE;
	VmaAllocation Allocation = VK_NULL_HANDLE;
};