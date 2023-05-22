#pragma once
#include <vulkan/vulkan_core.h>

#include "OpenRenderRuntime/Core/RHI/RHISampler.h"

class VulkanRHISampler : public RHISampler
{
    
public:
    VkSampler Sampler;
};