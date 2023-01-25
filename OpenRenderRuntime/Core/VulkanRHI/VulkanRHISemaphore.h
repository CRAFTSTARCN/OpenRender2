#pragma once
#include <vulkan/vulkan_core.h>

#include "OpenRenderRuntime/Core/RHI/RHISemaphore.h"

class VulkanRHISemaphore : public RHISemaphore
{
	
public:
	VkSemaphore VulkanSemaphore = VK_NULL_HANDLE;
};
