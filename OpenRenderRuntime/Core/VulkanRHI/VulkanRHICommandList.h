#pragma once
#include <mutex>
#include <queue>
#include <set>
#include <vulkan/vulkan_core.h>

#include "OpenRenderRuntime/Core/RHI/RHICommandList.h"


class VulkanCmdBufferStatusStruct
{
    
public:
    
    std::queue<int> FreeQueue;
    std::set<int> BusyQueue;
    std::queue<int> RunningQueue;
    std::mutex QueueSyncMutex;
};

class VulkanRHICommandList : public RHICommandList
{
    
public:

    int BufferIndex = -1;
};
