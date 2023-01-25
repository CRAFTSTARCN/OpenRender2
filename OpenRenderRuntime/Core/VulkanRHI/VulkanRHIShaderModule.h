#pragma once
#include <vulkan/vulkan_core.h>

#include "OpenRenderRuntime/Core/RHI/RHIShaderModule.h"

class VulkanRHIShaderModule : public RHIShaderModule
{
    
public:
    
    VkShaderModule ShaderModule = VK_NULL_HANDLE;
};
