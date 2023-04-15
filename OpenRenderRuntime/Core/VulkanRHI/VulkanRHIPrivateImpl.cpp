#include "OpenRenderRuntime/Core/VulkanRHI/VulkanRHI.h"
#include "OpenRenderRuntime/Core/VulkanRHI/VKTool.h"
#include "OpenRenderRuntime/Util/Logger.h"

#include <cassert>

#include "OpenRenderRuntime/Core/VulkanRHI/VulkanRHIDescriptor.h"
#include "OpenRenderRuntime/Core/VulkanRHI/VulkanRHIPipeline.h"
#include "OpenRenderRuntime/Core/VulkanRHI/VulkanRHIRenderPass.h"
#include "OpenRenderRuntime/Core/VulkanRHI/VulkanRHITexture.h"
#include "OpenRenderRuntime/Core/VulkanRHI/VulkanRHIBuffer.h"

VkPipelineStageFlags VulkanRHI::TransferPipelineStage(PipelineStage Stages)
{
    VkPipelineStageFlags Flags = 0;
    for(int i=0; i<=14; ++i)
    {
        if(Stages & 1 << i)
        {
            Flags |= PipelineStageTransfer[i];
        }
    }

    return Flags;
}

VkBufferUsageFlags VulkanRHI::TransferBufferUsage(BufferUsage BufferUsages)
{
    VkBufferUsageFlags InternalBufferUsage = 0;
    for(int i=0; i<=3; ++i)
    {
        if(BufferUsages & (1 << i))
        {
            InternalBufferUsage |= BufferUsageTransfer[i];
        }
    }

    return InternalBufferUsage;
}

VkImageUsageFlags VulkanRHI::TransferImageUsage(VkImageUsageFlags ImageUsage)
{
    VkImageUsageFlags ImageUsageFlags = 0;
    for(int i = 0; i<=7; ++i)
    {
        if(ImageUsage & (1 << i))
        {
            ImageUsageFlags |= TextureUsageBitTransfer[i];
        }
    }

    return ImageUsageFlags;
}

VkImageAspectFlags VulkanRHI::TransferAspectWithFormat(RHIFormat Format)
{
    if(Format == RHIFormat_D32_FLOAT)
    {
        return VK_IMAGE_ASPECT_DEPTH_BIT;
    }
    else if (Format == RHIFormat_D32_FLOAT_S8_UINT)
    {
        return VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
    }
    else if(Format == RHIFormat_S8_UINT)
    {
        return VK_IMAGE_ASPECT_STENCIL_BIT;
    }
    
    return VK_IMAGE_ASPECT_COLOR_BIT;
    
}

VkImageAspectFlags VulkanRHI::TransferAspect(TexturePlane Plane)
{
    VkImageUsageFlags UsageFlags = 0;
    if(Plane & TexturePlaneBit_Depth)
    {
        UsageFlags |= VK_IMAGE_ASPECT_DEPTH_BIT;
    }
    if(Plane & TexturePlaneBit_Stencil)
    {
        UsageFlags |= VK_IMAGE_ASPECT_STENCIL_BIT;
    }
    if(Plane &= TexturePlaneBit_Color)
    {
        UsageFlags |= VK_IMAGE_ASPECT_COLOR_BIT;
    }
    return UsageFlags;
}

void VulkanRHI::SetBufferDataDirect(VulkanRHIBuffer* Buffer, const void* Data, size_t Size, size_t Offset)
{
    uint8_t* StartPtr = ((uint8_t*)Buffer->MappedPtr) + Offset;
    memcpy(StartPtr, Data, Size);
    if(Buffer->MemoryUsage != BufferMemoryUsage_Host_Coherent)
    {
        vmaFlushAllocation(VulkanContext->Allocator, Buffer->Allocation, Offset, Size);
    }
}

void VulkanRHI::SetBufferDataStaging(VulkanRHIBuffer* Buffer, const void* Data, size_t Size, size_t Offset)
{
    VkBuffer StagingBuffer = VK_NULL_HANDLE;
    VkDeviceMemory StagingBufferMemory = VK_NULL_HANDLE;
    bool Result = VKTool::CreateStagingBuffer(VulkanContext, Size, StagingBuffer, StagingBufferMemory);
    if(!Result)
    {
        LOG_ERROR_FUNCTION("Create staging buffer error");
        return;
    }

    void* MappedPtr = nullptr;
    vkMapMemory(VulkanContext->Device, StagingBufferMemory, 0, Size, 0, &MappedPtr);
    memcpy(MappedPtr, Data, Size);
    vkUnmapMemory(VulkanContext->Device, StagingBufferMemory);

    VkCommandBuffer Cmd = CreateSingletTimeCommandBuffer();
    VKTool::CopyBuffer2Buffer(Cmd, StagingBuffer, Buffer->BufferObject, Size, 0, Offset);
    EndSingleTimeCommandBuffer(Cmd);

    EndStageBuffer(StagingBuffer, StagingBufferMemory);
}

int VulkanRHI::TryGetValidCommandIndex(VulkanCmdBufferStatusStruct& Struct)
{
    std::lock_guard<std::mutex> Guard(Struct.QueueSyncMutex);
    if(!Struct.FreeQueue.empty())
    {
        int Index = Struct.FreeQueue.front();
        Struct.FreeQueue.pop();
        Struct.BusyQueue.insert(Index);
        return Index;
    }

    return -1;
}

void VulkanRHI::TryEndCommandIndex(VulkanCmdBufferStatusStruct& Struct, int Index)
{
    auto Iter = Struct.BusyQueue.find(Index);
    if(Iter == Struct.BusyQueue.end())
    {
        LOG_ERROR("Invalid submit index");
        assert(false);
    }

    Struct.BusyQueue.erase(Index);
    Struct.RunningQueue.push(Index);
}

uint64_t VulkanRHI::EncodingSampler(const TextureSamplerCreateStruct& SampleInfo)
{
    uint64_t Code = 0;

    Code |= (uint64_t)SampleInfo.WrapU;
    Code |= ((uint64_t)SampleInfo.WrapV) << 2;
    Code |= ((uint64_t)SampleInfo.WrapW) << 4;

    Code |= ((uint64_t)SampleInfo.FilterMin) << 6;
    Code |= ((uint64_t)SampleInfo.FilterMag) << 8;

    Code |= ((uint64_t)SampleInfo.MipmapFilter) << 10;

    Code |= SampleInfo.Anisotropy << 11;

    Code |= ((uint64_t)SampleInfo.BorderColor[0]) << 32;
    Code |= ((uint64_t)SampleInfo.BorderColor[1]) << 40;
    Code |= ((uint64_t)SampleInfo.BorderColor[2]) << 48;
    Code |= ((uint64_t)SampleInfo.BorderColor[3]) << 56;
    
    return Code;
}

VkSampler VulkanRHI::GetSampler(const TextureSamplerCreateStruct& SampleInfo)
{
    uint64_t Code = EncodingSampler(SampleInfo);
    auto Iter = SamplerTable.find(Code);
    if(Iter != SamplerTable.end())
    {
        return Iter->second;
    }

    uint32_t CodedColor = (uint32_t)SampleInfo.BorderColor[0] |
                          (uint32_t)SampleInfo.BorderColor[1] << 8 |
                          (uint32_t)SampleInfo.BorderColor[2] << 16 |
                          (uint32_t)SampleInfo.BorderColor[3] << 24;
    
    VkSampler Sampler = VK_NULL_HANDLE;

    Sampler = VKTool::CreateVulkanSampler(
        VulkanContext,
        WrapTransfer[SampleInfo.WrapU],
        WrapTransfer[SampleInfo.WrapV],
        WrapTransfer[SampleInfo.WrapW],
        FilterTypeTransfer[SampleInfo.FilterMin],
        FilterTypeTransfer[SampleInfo.FilterMag],
        MipmapSampleTransfer[SampleInfo.MipmapFilter],
        SampleInfo.Anisotropy,
        CodedColor);
    
    if(Sampler == VK_NULL_HANDLE)
    {
        LOG_ERROR_FUNCTION("Fail to create sampler");
    }
    else
    {
        SamplerTable.emplace(Code, Sampler);
    }

    return Sampler;
}

void VulkanRHI::ClearRunningCommandBuffer(VulkanCmdBufferStatusStruct& Struct)
{
    std::queue<int>& RunningQueue = Struct.RunningQueue;
    std::queue<int>& FreeQueue = Struct.FreeQueue;

    while(!RunningQueue.empty())
    {
        FreeQueue.push(RunningQueue.front());
        RunningQueue.pop();
    }
}

void VulkanRHI::SyncRunningCommands()
{
    vkQueueWaitIdle(VulkanContext->ComputeQueue);
    vkQueueWaitIdle(VulkanContext->GraphicsQueue);
    vkQueueWaitIdle(VulkanContext->PresentQueue); //For safety, sync presentation
    
    ClearRunningCommandBuffer(ComputeCommandsStatus);
    ClearRunningCommandBuffer(DrawCommandsStatus);
}

void VulkanRHI::DestroyAllSampler()
{
    for(auto Sampler : SamplerTable)
    {
        vkDestroySampler(VulkanContext->Device, Sampler.second, nullptr);
    }
}
