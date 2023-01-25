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
    for(int i=0; i<=13; ++i)
    {
        if(Stages & 1 << i)
        {
            Flags |= PipelineStageTransfer[i];
        }
    }

    return Flags;
}

void VulkanRHI::SetBufferDataDirect(VulkanRHIBuffer* Buffer, const void* Data, size_t Size, size_t Offset)
{
    uint8_t* StartPtr = ((uint8_t*)Buffer->MappedPtr) + Offset;
    memcpy(StartPtr, Data, Size);
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

uint64_t VulkanRHI::EncodingSampler(uint32_t MipmapLevel, const TextureSamplerCreateStruct& SampleInfo,
                                    uint32_t Anisotropy)
{
    uint64_t Code = 0;

    Code |= (uint64_t)SampleInfo.WrapU;
    Code |= ((uint64_t)SampleInfo.WrapV) << 2;
    Code |= ((uint64_t)SampleInfo.WrapW) << 4;

    Code |= ((uint64_t)SampleInfo.FilterMin) << 6;
    Code |= ((uint64_t)SampleInfo.FilterMag) << 8;

    Code |= ((uint64_t)SampleInfo.MipmapFilter) << 10;

    Code |= MipmapLevel << 11;
    Code |= Anisotropy << 26;

    Code |= ((uint64_t)SampleInfo.BorderColor[0]) << 32;
    Code |= ((uint64_t)SampleInfo.BorderColor[1]) << 40;
    Code |= ((uint64_t)SampleInfo.BorderColor[2]) << 48;
    Code |= ((uint64_t)SampleInfo.BorderColor[3]) << 56;
    
    return Code;
}

VkSampler VulkanRHI::GetSampler(uint32_t MipmapLevel, const TextureSamplerCreateStruct& SampleInfo, uint32_t Anisotropy)
{
    uint64_t Code = EncodingSampler(MipmapLevel, SampleInfo, Anisotropy);
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
        MipmapLevel,
        Anisotropy,
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
