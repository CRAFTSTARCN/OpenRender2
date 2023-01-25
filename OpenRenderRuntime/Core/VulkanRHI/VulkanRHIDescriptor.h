#pragma once
#include <vulkan/vulkan_core.h>

#include "OpenRenderRuntime/Core/RHI/RHIDescriptor.h"

class VulkanRHIDescriptorLayout : public RHIDescriptorLayout
{
	
public:
	
	VkDescriptorSetLayout Layout = VK_NULL_HANDLE;
};

class VulkanRHIDescriptorSet : public RHIDescriptorSet
{
	
public:
	
	VkDescriptorSet Set = VK_NULL_HANDLE;
};