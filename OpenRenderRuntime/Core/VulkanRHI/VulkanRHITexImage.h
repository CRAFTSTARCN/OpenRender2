#pragma once
#include <vk_mem_alloc.h>
#include <vulkan/vulkan_core.h>

#include "OpenRenderRuntime/Core/RHI/RHITexImage.h"

class VulkanRHITexImage : public RHITexImage
{
	
public:

	VkImage Image = VK_NULL_HANDLE;
	VmaAllocation Allocation = VK_NULL_HANDLE;
};