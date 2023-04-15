#pragma once
#include <mutex>
#include <queue>
#include <set>
#include <vector>
#include <vulkan/vulkan_core.h>

#include "RHI/RHIEnums.h"

class VulkanContext;

/*
 * Flush type: thread safe
 */
class VulkanCommandPool
{

    VulkanContext* Context;
    
    VkCommandPool Pool;

    std::queue<VkCommandBuffer> FreeBuffers;
    std::set<VkCommandBuffer> BusyBuffers;

    RenderTaskType Type;

    std::mutex PoolAccessMutex;
    
    void CreateCommandPool();

    void AllocateCommandBuffer(uint32_t Count);
    
public:

    VulkanCommandPool(VulkanContext* InContext, RenderTaskType InType, uint32_t InitCommandBuffer);
    
    VkCommandBuffer GetOrCreateFreeBuffer();

    void ReturnBufferToFree(VkCommandBuffer BufferHandle);
};
