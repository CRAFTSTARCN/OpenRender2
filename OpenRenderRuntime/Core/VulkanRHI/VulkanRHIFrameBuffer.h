#pragma once
#include <vulkan/vulkan_core.h>

#include "OpenRenderRuntime/Core/RHI/RHIFrameBuffer.h"

class VulkanRHIFrameBuffer : public RHIFrameBuffer
{
	
public:

	VkFramebuffer FrameBuffer = VK_NULL_HANDLE;
	
};
