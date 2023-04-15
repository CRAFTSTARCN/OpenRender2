#include "VulkanCommandPool.h"

#include <assert.h>

#include "VulkanRHIContext.h"

void VulkanCommandPool::CreateCommandPool()
{
    VkCommandPoolCreateInfo DrawCommandPoolCreateInfo {};
    DrawCommandPoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    DrawCommandPoolCreateInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    if(Type == RenderTaskType::Graphics)
    {
        DrawCommandPoolCreateInfo.queueFamilyIndex = Context->GraphicsQueueFamilyIndex;
    }
    else if(Type == RenderTaskType::Compute)
    {
        DrawCommandPoolCreateInfo.queueFamilyIndex = Context->ComputeQueueFamilyIndex;
    }
    else
    {
        assert(false);
    }

    VkResult PoolCreateResult = vkCreateCommandPool(Context->Device, &DrawCommandPoolCreateInfo, nullptr, &Pool);
    assert(PoolCreateResult == VK_SUCCESS);
}

void VulkanCommandPool::AllocateCommandBuffer(uint32_t Count)
{
    std::vector<VkCommandBuffer> Cmd(Count);
    
    VkCommandBufferAllocateInfo CommandBufferInfo {};
    CommandBufferInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    CommandBufferInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    CommandBufferInfo.commandPool = Pool;
    CommandBufferInfo.commandBufferCount = Count;

    VkResult Result = vkAllocateCommandBuffers(Context->Device, &CommandBufferInfo, Cmd.data());
    assert(Result == VK_SUCCESS);

    for(auto CmdBuffer : Cmd)
    {
        FreeBuffers.push(CmdBuffer);
    }
}

VulkanCommandPool::VulkanCommandPool(VulkanContext* InContext, RenderTaskType InType, uint32_t InitCommandBuffer) :
    Context(InContext), Pool(VK_NULL_HANDLE), Type(InType)
{
    CreateCommandPool();
}

VkCommandBuffer VulkanCommandPool::GetOrCreateFreeBuffer()
{
    std::unique_lock<std::mutex> ScopeGuard(PoolAccessMutex);
    if(FreeBuffers.empty())
    {
        AllocateCommandBuffer(1);
    }

    VkCommandBuffer Top = FreeBuffers.front();
    FreeBuffers.pop();
    BusyBuffers.insert(Top);

    return Top;
}

void VulkanCommandPool::ReturnBufferToFree(VkCommandBuffer BufferHandle)
{
    std::unique_lock<std::mutex> ScopeGuard(PoolAccessMutex);
    if(auto Iter = BusyBuffers.find(BufferHandle); Iter != BusyBuffers.end())
    {
        BusyBuffers.erase(Iter);
        FreeBuffers.push(BufferHandle);
    }
}
