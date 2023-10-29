#pragma once
#include <vulkan/vulkan_core.h>

#include "OpenRenderRuntime/Core/RHI/RHIDescriptor.h"

class VulkanRHIDescriptorLayout : public RHIDescriptorLayout
{
	
public:
	
	VkDescriptorSetLayout Layout = VK_NULL_HANDLE;

	std::vector<VkDescriptorPoolSize> PoolSizeInfos;
};

class VulkanRHIDescriptorSet : public RHIDescriptorSet
{
	
public:

	VkDescriptorPool Pool = VK_NULL_HANDLE;
	
	VkDescriptorSet Set = VK_NULL_HANDLE;
};