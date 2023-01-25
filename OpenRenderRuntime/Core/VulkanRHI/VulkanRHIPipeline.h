#pragma once
#include <vulkan/vulkan_core.h>

#include "OpenRenderRuntime/Core/RHI/RHIPipeline.h"

class VulkanRHIPipeline : public RHIPipeline
{
	
public:
	VkPipeline Pipeline = VK_NULL_HANDLE;
	VkPipelineLayout Layout = VK_NULL_HANDLE;
};
