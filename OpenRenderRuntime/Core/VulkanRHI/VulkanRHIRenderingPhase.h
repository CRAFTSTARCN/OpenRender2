#pragma once
#include <mutex>
#include <queue>
#include <set>
#include <vulkan/vulkan_core.h>

#include "OpenRenderRuntime/Core/RHI/RHIRenderingPhase.h"


class VulkanCmdBufferStatusStruct
{
    
public:
    
    std::queue<int> FreeQueue;
    std::set<int> BusyQueue;
    std::queue<int> RunningQueue;
    std::mutex QueueSyncMutex;
};

class VulkanPhaseSubmitInfo
{
    
public:
    VkQueue SubmitQueue = VK_NULL_HANDLE;
    VkCommandBuffer SubmitBuffer = VK_NULL_HANDLE;
    std::vector<VkSemaphore> WaitSemaphore {};
    std::vector<VkPipelineStageFlags> WaitStage {};
    std::vector<VkSemaphore> SignalSemaphore {};
};

class VulkanRHIRenderingPhase : public RHIRenderingPhase
{
    
public:

    int BufferIndex = -1;
};
