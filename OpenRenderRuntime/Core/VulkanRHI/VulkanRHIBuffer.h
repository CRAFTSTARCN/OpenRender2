#pragma once
#include <vk_mem_alloc.h>
#include <vulkan/vulkan_core.h>

#include "OpenRenderRuntime/Core/RHI/RHIBuffer.h"

class VulkanRHIBuffer : public RHIBuffer
{
    
public:
    VkBuffer BufferObject = VK_NULL_HANDLE;
    VmaAllocation Allocation = VK_NULL_HANDLE;
    void* MappedPtr = nullptr;
};