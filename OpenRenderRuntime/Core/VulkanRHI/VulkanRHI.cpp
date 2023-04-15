#include "OpenRenderRuntime/Core/VulkanRHI/VulkanRHI.h"
#include "OpenRenderRuntime/Core/VulkanRHI/VKTool.h"
#include "OpenRenderRuntime/Modules/RenderConfig/RenderCofig.h"
#include "OpenRenderRuntime/Util/Logger.h"

#include <cassert>

#include "OpenRenderRuntime/Core/VulkanRHI/VulkanRHIFrameBuffer.h"
#include "OpenRenderRuntime/Core/VulkanRHI/VulkanRHISampler.h"
#include "OpenRenderRuntime/Core/VulkanRHI/VulkanRHISemaphoreView.h"
#include "OpenRenderRuntime/Core/VulkanRHI/VulkanRHIDescriptor.h"
#include "OpenRenderRuntime/Core/VulkanRHI/VulkanRHIPipeline.h"
#include "OpenRenderRuntime/Core/VulkanRHI/VulkanRHIRenderTarget.h"
#include "OpenRenderRuntime/Core/VulkanRHI/VulkanRHIRenderPass.h"
#include "OpenRenderRuntime/Core/VulkanRHI/VulkanRHIRenderSubpass.h"
#include "OpenRenderRuntime/Core/VulkanRHI/VulkanRHITexImage.h"
#include "OpenRenderRuntime/Core/VulkanRHI/VulkanRHITextureView.h"
#include "OpenRenderRuntime/Core/VulkanRHI/VulkanRHISemaphore.h"
#include "OpenRenderRuntime/Core/VulkanRHI/VulkanRHITexture.h"
#include "OpenRenderRuntime/Core/VulkanRHI/VulkanRHIBuffer.h"
#include "OpenRenderRuntime/Core/VulkanRHI/VulkanRHIShaderModule.h"

#define RHI_2_VK_CHECKED(VulkanType, RHIComponent, VulkanComponentName) \
    VulkanType *(VulkanComponentName) = (VulkanType*)(RHIComponent);\
    if(!(VulkanComponentName)) {\
        LOG_ERROR_FUNCTION("Null rhi stuff passed to rhi function, type {0}, rest stuff will not run", #VulkanType);\
        return;\
    }

#define RHI_2_VK_CHECKED_NULL(VulkanType, RHIComponent, VulkanComponentName) \
    VulkanType *(VulkanComponentName) = (VulkanType*)(RHIComponent);\
    if(!(VulkanComponentName)) {\
        LOG_ERROR_FUNCTION("Null rhi stuff passed to rhi function, type {0}, null will returned", #VulkanType);\
        return nullptr;\
    }

void VulkanRHI::PreInit()
{
    RenderConfig& Config = RenderConfig::Get();

    assert(glfwInit());

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    
    WindowPtr = glfwCreateWindow(
        Config.RenderWindowWidth,
        Config.RenderWindowHeight,
        Config.RenderWindowName.c_str(),
        nullptr, nullptr);
}

void VulkanRHI::Init()
{
    SetupContext();
    InitializeVulkan();
}

void VulkanRHI::PreTerminate()
{
    vkDeviceWaitIdle(VulkanContext->Device);
}

void VulkanRHI::Terminate()
{
    DestroyAllSampler();
    DestroyVulkanObjects();
    delete VulkanContext;
    glfwDestroyWindow(WindowPtr);

    glfwTerminate();
}

void VulkanRHI::SetupContext()
{
    VulkanContext = new VulkanRHIContext;
    Context = VulkanContext;
    VulkanContext->EnableDebug = RenderConfig::Get().VKEnableValidationLayer;
}

RHIShaderModule* VulkanRHI::CreateVertexShader(const void* ShaderSource, size_t Size)
{
    VkShaderModule SM = VKTool::CreateShaderModuleInternal(VulkanContext, ShaderSource, Size);
    if(SM == VK_NULL_HANDLE)
    {
        return nullptr;
    }

    return new VulkanRHIShaderModule{{ShaderStageTypeBit_Vertex}, SM};
}

RHIShaderModule* VulkanRHI::CreateFragmentShader(const void* ShaderSource, size_t Size)
{
    VkShaderModule SM = VKTool::CreateShaderModuleInternal(VulkanContext, ShaderSource, Size);
    if(SM == VK_NULL_HANDLE)
    {
        return nullptr;
    }

    return new VulkanRHIShaderModule{{ShaderStageTypeBit_Fragment}, SM};
}

RHIShaderModule* VulkanRHI::CreateMeshShader(const void* ShaderSource, size_t Size)
{
    VkShaderModule SM = VKTool::CreateShaderModuleInternal(VulkanContext, ShaderSource, Size);
    if(SM == VK_NULL_HANDLE)
    {
        return nullptr;
    }

    return new VulkanRHIShaderModule{{ShaderStageTypeBit_Mesh}, SM};
}

RHIShaderModule* VulkanRHI::CreateTaskShader(const void* ShaderSource, size_t Size)
{
    VkShaderModule SM = VKTool::CreateShaderModuleInternal(VulkanContext, ShaderSource, Size);
    if(SM == VK_NULL_HANDLE)
    {
        return nullptr;
    }

    return new VulkanRHIShaderModule{{ShaderStageTypeBit_Task}, SM};
}

RHIShaderModule* VulkanRHI::CreateComputeShader(const void* ShaderSource, size_t Size)
{
    VkShaderModule SM = VKTool::CreateShaderModuleInternal(VulkanContext, ShaderSource, Size);
    if(SM == VK_NULL_HANDLE)
    {
        return nullptr;
    }

    return new VulkanRHIShaderModule{{ShaderStageTypeBit_Compute}, SM};
}

RHIBuffer* VulkanRHI::CreateStorageBuffer(size_t Size, BufferMemoryUsage Usage)
{
    VmaMemoryUsage InternalUsage = BufferMemoryUsageTransfer[Usage];
    VkBufferUsageFlags InternalBufferUsage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;

    if(Usage == BufferMemoryUsage_GPU_Only || Usage == BufferMemoryUsage_CPU_Copy )
    {
        InternalBufferUsage |= VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    }
    
    VkBuffer Buffer = VK_NULL_HANDLE;
    VmaAllocation BufferAllocation = VK_NULL_HANDLE;
    bool Result = VKTool::CreateBuffer(
        VulkanContext,
        Size,
        InternalBufferUsage,
        InternalUsage,
        Buffer,
        BufferAllocation);

    if(!Result)
    {
        return nullptr;
    }

    VulkanRHIBuffer *BufferPtr =  new VulkanRHIBuffer{{Usage}, Buffer, BufferAllocation};
    if(Usage != BufferMemoryUsage_GPU_Only)
    {
        vmaMapMemory(VulkanContext->Allocator, BufferAllocation, &BufferPtr->MappedPtr);
    }

    return BufferPtr;
}

RHIBuffer* VulkanRHI::CreateUniformBuffer(size_t Size)
{
    VkBuffer Buffer = VK_NULL_HANDLE;
    VmaAllocation BufferAllocation = VK_NULL_HANDLE;
    bool Result = VKTool::CreateBuffer(
        VulkanContext,
        Size,
        VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT ,
        VMA_MEMORY_USAGE_CPU_TO_GPU,
        Buffer,
        BufferAllocation);
    if(!Result)
    {
        return nullptr;
    }

    VulkanRHIBuffer *BufferPtr =  new VulkanRHIBuffer{{BufferMemoryUsage_CPU_2_GPU}, Buffer, BufferAllocation};
    vmaMapMemory(VulkanContext->Allocator, BufferAllocation, &BufferPtr->MappedPtr);
    

    return BufferPtr;
}

RHIBuffer* VulkanRHI::CreateVertexBuffer(size_t Size)
{
    VkBuffer Buffer = VK_NULL_HANDLE;
    VmaAllocation BufferAllocation = VK_NULL_HANDLE;
    bool Result = VKTool::CreateBuffer(
        VulkanContext,
        Size,
        VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
        VMA_MEMORY_USAGE_GPU_ONLY,
        Buffer,
        BufferAllocation);
    if(!Result)
    {
        return nullptr;
    }

    VulkanRHIBuffer *BufferPtr =  new VulkanRHIBuffer{{BufferMemoryUsage_GPU_Only}, Buffer, BufferAllocation};
    return BufferPtr;
}

RHIBuffer* VulkanRHI::CreateIndexBuffer(size_t Size)
{
    VkBuffer Buffer = VK_NULL_HANDLE;
    VmaAllocation BufferAllocation = VK_NULL_HANDLE;
    bool Result = VKTool::CreateBuffer(
        VulkanContext,
        Size,
        VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
        VMA_MEMORY_USAGE_GPU_ONLY,
        Buffer,
        BufferAllocation);
    if(!Result)
    {
        return nullptr;
    }

    VulkanRHIBuffer *BufferPtr =  new VulkanRHIBuffer{{BufferMemoryUsage_GPU_Only}, Buffer, BufferAllocation};
    return BufferPtr;
}

RHIBuffer* VulkanRHI::CreateAnyBuffer(size_t Size, BufferUsage BufferUsage, BufferMemoryUsage Usage, bool Share)
{
    VmaMemoryUsage InternalUsage = BufferMemoryUsageTransfer[Usage];
    VkBufferUsageFlags InternalBufferUsage = TransferBufferUsage(BufferUsage);

    VkBuffer Buffer = VK_NULL_HANDLE;
    VmaAllocation BufferAllocation = VK_NULL_HANDLE;
    bool Result = VKTool::CreateBuffer(
        VulkanContext,
        Size,
        InternalBufferUsage,
        InternalUsage,
        Buffer,
        BufferAllocation);

    if(!Result)
    {
        return nullptr;
    }

    VulkanRHIBuffer *BufferPtr =  new VulkanRHIBuffer{{Usage}, Buffer, BufferAllocation};
    if(Usage != BufferMemoryUsage_GPU_Only)
    {
        vmaMapMemory(VulkanContext->Allocator, BufferAllocation, &BufferPtr->MappedPtr);
    }

    return BufferPtr;
}


void VulkanRHI::SetBufferData(RHIBuffer* Buffer, const void* Data, size_t Size, size_t Offset)
{
    RHI_2_VK_CHECKED(VulkanRHIBuffer, Buffer, VulkanBuffer)
    if(Buffer->MemoryUsage == BufferMemoryUsage_GPU_Only || Buffer->MemoryUsage == BufferMemoryUsage_CPU_Copy)
    {
        SetBufferDataStaging(VulkanBuffer, Data, Size, Offset);
    }
    else
    {
        SetBufferDataDirect(VulkanBuffer, Data, Size, Offset);
    }
}

uint32_t VulkanRHI::GetSwapchainImageCount() const
{
    return (uint32_t)VulkanContext->SwapchainImages.size();
}

RHIFormat VulkanRHI::GetSwapchainTextureFormat() const
{
    static std::unordered_map<VkFormat, RHIFormat> FormatInternalTransfer = {
        {VK_FORMAT_R8G8B8_UNORM, RHIFormat_RGB8},
        {VK_FORMAT_B8G8R8A8_UNORM,RHIFormat_BGRA8},
        {VK_FORMAT_B8G8R8A8_SRGB, RHIFormat_BGRA8_SRGB},
        {VK_FORMAT_R16G16B16_SFLOAT, RHIFormat_RGBA16_FLOAT},
        {VK_FORMAT_A2B10G10R10_UNORM_PACK32, RHIFormat_A2_BGR10_HDR}
    };
    auto Iter = FormatInternalTransfer.find(VulkanContext->Format);
    if(Iter == FormatInternalTransfer.end())
    {
        LOG_ERROR_FUNCTION("Unsupport swapchain format");
        assert(false);
    }

    return Iter->second;
}

uint32_t VulkanRHI::GetSwapchainExtendWidth()
{
    return VulkanContext->Extend.width;
}

uint32_t VulkanRHI::GetSwapchainExtendHeight()
{
    return VulkanContext->Extend.height;
}

RHITexture& VulkanRHI::GetSwapchainTexture(uint32_t Index)
{
    if(Index >= DefaultAttachments.size())
    {
        LOG_ERROR_FUNCTION("Invalid index for swapchain attachment");
        assert(false);
    }

    return *DefaultAttachments[Index];
}

uint32_t VulkanRHI::GetCurrentSwapchainImageIndex()
{
    return VulkanContext->CurrentFrameIndex;
}

RHICommandList* VulkanRHI::GetCommandList(RenderingTaskQueue PhaseQueue)
{
    VulkanRHICommandList *Result = new VulkanRHICommandList{{PhaseQueue}, -1};
    VkResult Reset = VK_SUCCESS; 
    if(PhaseQueue == RenderingTaskQueue_Graphics)
    {
        Result->BufferIndex = TryGetValidCommandIndex(DrawCommandsStatus);
    }
    else
    {
        Result->BufferIndex = TryGetValidCommandIndex(ComputeCommandsStatus);

    }
    
    if(Result->BufferIndex < 0)
    {
        LOG_ERROR_FUNCTION("No valid command buffer left");
        assert(false);
    }

    VkCommandBuffer Buffer = PhaseQueue == RenderingTaskQueue_Graphics ?
                                           VulkanContext->DrawCommandBuffers[Result->BufferIndex] :
                                           VulkanContext->ComputeCommandBuffers[Result->BufferIndex];
    
    
    Reset = vkResetCommandBuffer(Buffer, VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT);
    
    if(Reset != VK_SUCCESS)
    {
        LOG_ERROR_FUNCTION("Fail to reset command buffer");
        assert(false);
    }

    VkCommandBufferBeginInfo BufferBegin {};
    BufferBegin.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    BufferBegin.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    VkResult BeginResult = vkBeginCommandBuffer(Buffer, &BufferBegin);

    return Result;
}

void VulkanRHI::SubmitCommandList(RHICommandList*& SubmittedPhase, const std::vector<RHISemaphoreView*>& WaitSemaphore,
                                     const std::vector<RHISemaphore*>& SignalSemaphore)
{
    RHI_2_VK_CHECKED(VulkanRHICommandList, SubmittedPhase, VulkanSubmittedPhase)
    VkQueue SubmitTo = VulkanContext->GraphicsQueue;
    VkCommandBuffer Buffer = VK_NULL_HANDLE;
    VulkanCmdBufferStatusStruct* Status = &DrawCommandsStatus;
    
    if(VulkanSubmittedPhase->PhaseQueue == RenderingTaskQueue_Graphics)
    {
        Buffer = VulkanContext->DrawCommandBuffers[VulkanSubmittedPhase->BufferIndex];
        vkEndCommandBuffer(Buffer);
    }
    else
    {
        Buffer = VulkanContext->ComputeCommandBuffers[VulkanSubmittedPhase->BufferIndex];
        Status = &ComputeCommandsStatus;
        vkEndCommandBuffer(Buffer);
        SubmitTo = VulkanContext->ComputeQueue;
    }

    std::vector<VkSemaphore> WaitVKSemaphore;
    std::vector<VkPipelineStageFlags> WaitStages;

    for(auto Semaphore : WaitSemaphore)
    {
        RHI_2_VK_CHECKED(VulkanRHISemaphoreView, Semaphore, VKSemaphoreView)
        RHI_2_VK_CHECKED(VulkanRHISemaphore, VKSemaphoreView->BaseSemaphore, VKSemaphore)
        WaitVKSemaphore.push_back(VKSemaphore->VulkanSemaphore);
        VkPipelineStageFlags WaitStage = TransferPipelineStage(Semaphore->WaitStage);
        WaitStages.push_back(WaitStage);
    }

    std::vector<VkSemaphore> SignalVKSemaphores {};

    for(auto Semaphore : SignalSemaphore)
    {
        RHI_2_VK_CHECKED(VulkanRHISemaphore, Semaphore, VKSemaphore)
        SignalVKSemaphores.push_back(VKSemaphore->VulkanSemaphore);
    }
    SubmittedPhase = nullptr;

    VkSubmitInfo SubmitInfo {};
    SubmitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    SubmitInfo.commandBufferCount = 1;
    SubmitInfo.pCommandBuffers = &Buffer;
    
    SubmitInfo.waitSemaphoreCount = (uint32_t)WaitVKSemaphore.size();
    SubmitInfo.pWaitSemaphores = WaitVKSemaphore.data();
    SubmitInfo.pWaitDstStageMask = WaitStages.data();
    SubmitInfo.signalSemaphoreCount = (uint32_t)SignalVKSemaphores.size();
    SubmitInfo.pSignalSemaphores = SignalVKSemaphores.data();

    std::lock_guard<std::mutex> LockGuard(Status->QueueSyncMutex);
    TryEndCommandIndex(*Status, VulkanSubmittedPhase->BufferIndex);
    VkResult SubmitStatus = vkQueueSubmit(SubmitTo, 1, &SubmitInfo, VK_NULL_HANDLE);
    if(SubmitStatus != VK_SUCCESS)
    {
        LOG_ERROR_FUNCTION("FATAL ERROR: fail to submit command buffer, buffer id {0}", (void*)Buffer);
    }
}

RHISemaphore* VulkanRHI::GetRenderStartSemaphore()
{
    
    return new VulkanRHISemaphore{{}, VulkanContext->ImageRequireSemaphore};
}

RHISemaphore* VulkanRHI::GetRenderEndSemaphore()
{
    return new VulkanRHISemaphore{{}, VulkanContext->PresentSemaphore};
}

RHISemaphore* VulkanRHI::CreateSemaphore()
{
    VkSemaphoreCreateInfo SemaphoreInfo {};
    SemaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkSemaphore Semaphore = VK_NULL_HANDLE;
    VkResult Result = vkCreateSemaphore(
        VulkanContext->Device,
        &SemaphoreInfo,
        nullptr,
        &Semaphore);

    if(Result != VK_SUCCESS)
    {
        LOG_ERROR_FUNCTION("Fail to create vulkan semaphore");
        return nullptr;
    }

    return new VulkanRHISemaphore{{}, Semaphore};
}

RHISemaphoreView* VulkanRHI::CreateSemaphoreView(RHISemaphore* Semaphore, PipelineStage WaitStage)
{
    return new VulkanRHISemaphoreView{{Semaphore, WaitStage}};
}

RHIRenderPass* VulkanRHI::CreateRenderPass(const std::vector<AttachmentDescription>& Attachments,
                                           const std::vector<SubpassInfo>& SubpassInfos)
{
    std::vector<VkAttachmentDescription> AttachmentDescriptions(Attachments.size());
    for(size_t i=0; i<Attachments.size(); ++i)
    {
        VkAttachmentDescription& Description = AttachmentDescriptions[i];
        const AttachmentDescription& Attachment = Attachments[i];
        Description = {};
        Description.format = FormatTransfer[Attachment.Format];
        Description.samples = VK_SAMPLE_COUNT_1_BIT;
        Description.loadOp = (VkAttachmentLoadOp)Attachment.Onload;
        Description.storeOp = (VkAttachmentStoreOp)Attachment.OnStore;
        Description.stencilLoadOp = (VkAttachmentLoadOp)Attachment.OnStencilLoad;
        Description.stencilStoreOp = (VkAttachmentStoreOp)Attachment.OnStencilStore;
        Description.initialLayout = (VkImageLayout)Attachment.InitialLayout;
        Description.finalLayout = (VkImageLayout)Attachment.FinalLayout;
    }

    std::vector<VkSubpassDescription> SubpassDescriptions(SubpassInfos.size());
    std::vector<VkSubpassDependency> Dependencies {};
    std::vector<VkAttachmentReference> AttachmentReferences {};
    

    for(size_t i=0; i<SubpassInfos.size(); ++i)
    {
        VkSubpassDescription& Description = SubpassDescriptions[i];
        const SubpassInfo& Info = SubpassInfos[i];
        Description = {};
        Description.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        Description.inputAttachmentCount = (uint32_t)Info.InputAttachment.size();
        Description.colorAttachmentCount = (uint32_t)Info.ColorAttachment.size();
        /*
         * First input attachment then color, then depth stencil
         */
        for(const auto& AttachmentRef : Info.InputAttachment)
        {
            AttachmentReferences.push_back({AttachmentRef.Index, (VkImageLayout)AttachmentRef.RequireLayout});
        }

        for(const auto& AttachmentRef : Info.ColorAttachment)
        {
            AttachmentReferences.push_back({AttachmentRef.Index, (VkImageLayout)AttachmentRef.RequireLayout});
        }
        if(Info.RequireDepthStencil)
        {
            AttachmentReferences.push_back({Info.DepthStencilAttachment.Index, (VkImageLayout)Info.DepthStencilAttachment.RequireLayout});
        }
        for(const auto& Dependency : Info.Dependency)
        {
            VkSubpassDependency VKDependency {};
            VKDependency.srcSubpass = Dependency.DependPass;
            VKDependency.dstSubpass = (uint32_t)i;
            VKDependency.srcStageMask = TransferPipelineStage(Dependency.SrcStage);
            VKDependency.dstStageMask = TransferPipelineStage(Dependency.DstStage);
            VKDependency.dependencyFlags = Dependency.InRegion ? VK_DEPENDENCY_BY_REGION_BIT : 0;
            Dependencies.push_back(VKDependency);
        }

        for(const auto Dependency : Info.ExternalDependency)
        {
            VkSubpassDependency VKDependency {};
            VKDependency.srcSubpass = (uint32_t)i;
            VKDependency.dstSubpass = VK_SUBPASS_EXTERNAL;
            VKDependency.srcStageMask = TransferPipelineStage(Dependency.SrcStage);
            VKDependency.dstStageMask = TransferPipelineStage(Dependency.DstStage);
            VKDependency.dependencyFlags = Dependency.InRegion ? VK_DEPENDENCY_BY_REGION_BIT : 0;
            Dependencies.push_back(VKDependency);
        }
    }

    uint32_t CurrentAttachmentRefOffset = 0;
    uint32_t DescriptionIndex = 0;
    for(auto& Description : SubpassDescriptions)
    {
        Description.pInputAttachments = &AttachmentReferences[CurrentAttachmentRefOffset];
        CurrentAttachmentRefOffset += Description.inputAttachmentCount;
        Description.pColorAttachments = &AttachmentReferences[CurrentAttachmentRefOffset];
        CurrentAttachmentRefOffset += Description.colorAttachmentCount;
        if(SubpassInfos[DescriptionIndex].RequireDepthStencil)
        {
            Description.pDepthStencilAttachment = &AttachmentReferences[CurrentAttachmentRefOffset];
            ++CurrentAttachmentRefOffset;
        }
        else
        {
            Description.pDepthStencilAttachment = nullptr;
        }
        ++DescriptionIndex;
    }

    VkRenderPassCreateInfo RenderPassInfo {};
    RenderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    RenderPassInfo.attachmentCount = (uint32_t)AttachmentDescriptions.size();
    RenderPassInfo.pAttachments = AttachmentDescriptions.data();
    RenderPassInfo.subpassCount = (uint32_t)SubpassDescriptions.size();
    RenderPassInfo.pSubpasses = SubpassDescriptions.data();
    RenderPassInfo.dependencyCount = (uint32_t)Dependencies.size();
    RenderPassInfo.pDependencies = Dependencies.data();

    VkRenderPass RenderPass = VK_NULL_HANDLE;
    VkResult Result = vkCreateRenderPass(VulkanContext->Device, &RenderPassInfo, nullptr, &RenderPass);
    if(Result != VK_SUCCESS)
    {
        LOG_ERROR_FUNCTION("Fail to create render pass");
        return nullptr;
    }

    VulkanRHIRenderPass* VKRRenderPass = new VulkanRHIRenderPass{{}, RenderPass};
    for(size_t i=0; i<SubpassInfos.size(); ++i)
    {
        VKRRenderPass->Passes.push_back(new VulkanRHIRenderSubpass{{}, (uint32_t)i});
    }

    return VKRRenderPass;
}

RHIDescriptorLayout* VulkanRHI::CreateDescriptorLayout(const std::vector<DescriptorBindingInfo>& BindingInfos)
{
    std::vector<VkDescriptorSetLayoutBinding> Bindings(BindingInfos.size());

    for(size_t i=0; i<BindingInfos.size(); ++i)
    {
        VkDescriptorSetLayoutBinding &Binding = Bindings[i];
        const DescriptorBindingInfo &BindingInfo = BindingInfos[i];
        Binding = {};
        
        Binding.binding = (uint32_t)i;
        Binding.descriptorCount = 1;
        Binding.descriptorType = DescriptorTypeTransfer[BindingInfo.Type];
        Binding.stageFlags = BindingInfo.UsageStage;
        Binding.pImmutableSamplers = nullptr;
    }

    VkDescriptorSetLayoutCreateInfo LayoutInfo {};
    LayoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    LayoutInfo.bindingCount = (uint32_t)BindingInfos.size();
    LayoutInfo.pBindings = Bindings.data();

    VkDescriptorSetLayout Layout = VK_NULL_HANDLE;
    VkResult Result = vkCreateDescriptorSetLayout(VulkanContext->Device, &LayoutInfo, nullptr, &Layout);
    if(Result != VK_SUCCESS)
    {
        return nullptr;
    }

    return new VulkanRHIDescriptorLayout{{}, Layout};
}

RHIDescriptorSet* VulkanRHI::CreateDescriptorSet(RHIDescriptorLayout* RHILayout)
{
    RHI_2_VK_CHECKED_NULL(VulkanRHIDescriptorLayout, RHILayout, VKRHILayout)
    VkDescriptorSetAllocateInfo DescriptorSetInfo {};
    DescriptorSetInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    DescriptorSetInfo.descriptorSetCount = 1;
    DescriptorSetInfo.pSetLayouts = &VKRHILayout->Layout;
    DescriptorSetInfo.descriptorPool = VulkanContext->DescriptorPool;

    VkDescriptorSet Set = VK_NULL_HANDLE;
    VkResult Result = vkAllocateDescriptorSets(VulkanContext->Device, &DescriptorSetInfo, &Set);
    if(Result != VK_SUCCESS)
    {
        return nullptr;
    }

    return new VulkanRHIDescriptorSet{{}, Set};
}

void VulkanRHI::WriteDescriptorSetMulti(RHIDescriptorSet* WriteSet, const std::vector<TextureWithSamplerWriteInfo>& Textures,
    const std::vector<ImageWriteInfo>& Images, const std::vector<BufferWriteInfo>& Buffers)
{
    RHI_2_VK_CHECKED(VulkanRHIDescriptorSet, WriteSet, VKSet)
    size_t TotalWrite = Textures.size() + Images.size() + Buffers.size();
    std::vector<VkDescriptorImageInfo> TextureImageInfos(Textures.size());
    std::vector<VkDescriptorImageInfo> ImageInfos(Images.size());
    std::vector<VkDescriptorBufferInfo> BufferInfos(Buffers.size());
    std::vector<VkWriteDescriptorSet> WriteInfo(TotalWrite);

    uint32_t WriteIndex = 0;
    for(size_t i=0; i<Textures.size(); ++i)
    {
        VkDescriptorImageInfo& TextureImageInfo = TextureImageInfos[i];
        RHI_2_VK_CHECKED(VulkanRHITextureView,Textures[i].Texture, VKTexImageView)
        RHI_2_VK_CHECKED(VulkanRHISampler, Textures[i].Sampler, VKSampler)
        TextureImageInfo.imageView = VKTexImageView->ImageView;
        TextureImageInfo.sampler = VKSampler->Sampler;
        TextureImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

        VkWriteDescriptorSet& TexWrite = WriteInfo[WriteIndex];
        TexWrite = VKTool::CreateDescriptorWrite(VKSet->Set, Textures[i].Binding, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
        TexWrite.pImageInfo = &TextureImageInfo;
        ++WriteIndex;
    }

    for(size_t i=0; i<Images.size(); ++i)
    {
        VkDescriptorImageInfo& ImageInfo = ImageInfos[i];
        RHI_2_VK_CHECKED(VulkanRHITextureView,Images[i].ImageView, VKTexImageView)
        ImageInfo.imageView = VKTexImageView->ImageView;
        ImageInfo.sampler = VK_NULL_HANDLE;
        ImageInfo.imageLayout = (VkImageLayout)Images[i].ImageLayout;

        VkDescriptorType Type = DescriptorTypeTransfer[Images[i].ImageType];

        if(Images[i].ImageType != DescriptorType_Storage_Image && Images[i].ImageType != DescriptorType_Input_Attachment)
        {
            LOG_ERROR_FUNCTION("Unsupported image wirte descriptor type");
        }

        VkWriteDescriptorSet& ImageWrite = WriteInfo[WriteIndex];
        ImageWrite = VKTool::CreateDescriptorWrite(VKSet->Set, Images[i].Binding, Type);
        ImageWrite.pImageInfo = &ImageInfo;
        ++WriteIndex;
    }

    for(size_t i=0; i<Buffers.size(); ++i)
    {
        VkDescriptorBufferInfo& BufferInfo = BufferInfos[i];
        RHI_2_VK_CHECKED(VulkanRHIBuffer, Buffers[i].Buffer, VKBuffer)
        BufferInfo.buffer = VKBuffer->BufferObject;
        BufferInfo.offset = Buffers[i].Offset;
        BufferInfo.range = Buffers[i].Range;
        
        VkDescriptorType Type = DescriptorTypeTransfer[Buffers[i].BufferType];
        if(Buffers[i].BufferType != DescriptorType_Uniform_Buffer && Buffers[i].BufferType != DescriptorType_Uniform_Buffer_Dynamic &&
            Buffers[i].BufferType != DescriptorType_Storage_Buffer && Buffers[i].BufferType != DescriptorType_Storage_Buffer_Dynamic)
        {
            LOG_ERROR_FUNCTION("Unsupport descriptor buffer type");
        }

        VkWriteDescriptorSet& BufferWrite = WriteInfo[WriteIndex];
        BufferWrite = VKTool::CreateDescriptorWrite(VKSet->Set, Buffers[i].Binding, Type);
        BufferWrite.pBufferInfo = &BufferInfo;
        ++WriteIndex;
    }

    vkUpdateDescriptorSets(VulkanContext->Device, (uint32_t)WriteInfo.size(), WriteInfo.data(), 0 ,nullptr);
}

RHIPipeline* VulkanRHI::CreateGraphicsPipeline(RHIRenderPass* Pass, uint32_t Subpass,
                                               const std::vector<RHIDescriptorLayout*>& Descriptors, std::vector<RHIShaderModule*> ShaderModules, const GraphicsPipelineFixedFunctionInfo& PipelineInfo)
{
    RHI_2_VK_CHECKED_NULL(VulkanRHIRenderPass, Pass, VKPass)
    std::vector<VkDescriptorSetLayout> Layouts;

    for(auto Descriptor : Descriptors)
    {
        RHI_2_VK_CHECKED_NULL(VulkanRHIDescriptorLayout, Descriptor, VKLayout)
        Layouts.push_back(VKLayout->Layout);
    }

    /*
     * Pipeline layout
     */
    VkPipelineLayout PipelineLayout = VKTool::CreatePipelineLayout(VulkanContext, Layouts);
    if(PipelineLayout == VK_NULL_HANDLE)
    {
        return nullptr;
    }

    /*
     * Shader stage
     */
    std::vector<VkPipelineShaderStageCreateInfo> ShaderStages;
    for(auto Shader : ShaderModules)
    {
        RHI_2_VK_CHECKED_NULL(VulkanRHIShaderModule, Shader, VKShader);
        ShaderStages.push_back(VKTool::CreateShaderStageInfo(VKShader->ShaderModule, (VkShaderStageFlagBits)Shader->ShaderType));
    }

    /*
     * Viewport scissor, currently fixed
     */
    VkPipelineViewportStateCreateInfo ViewportScissorInfo {};
    ViewportScissorInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    ViewportScissorInfo.viewportCount = 1;
    ViewportScissorInfo.pViewports = &VulkanContext->DefaultViewport;
    ViewportScissorInfo.scissorCount = 1;
    ViewportScissorInfo.pScissors = &VulkanContext->DefaultScissor;

    /*
     * Rasterization
     */
    VkPipelineRasterizationStateCreateInfo RasterizationStageInfo = VKTool::CreateRasterizationInfo(
        (VkPolygonMode)PipelineInfo.RasterizationFill,
        PipelineInfo.RasterizationCullFace,
        (VkFrontFace)PipelineInfo.Front,
        PipelineInfo.LineWidth);

    /*
     * Multisample, not support yet
     */
    VkPipelineMultisampleStateCreateInfo MultisampleInfo {};
    MultisampleInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    MultisampleInfo.sampleShadingEnable  = VK_FALSE;
    MultisampleInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

    std::vector<VkPipelineColorBlendAttachmentState> ColorBlendingAttachments(PipelineInfo.ColorBlendingInfos.size());
    for(size_t i=0; i<ColorBlendingAttachments.size(); ++i)
    {
        const ColorBlendingInfo& Info = PipelineInfo.ColorBlendingInfos[i];
        VkPipelineColorBlendAttachmentState& ColorBlendingState = ColorBlendingAttachments[i];
        ColorBlendingState = VKTool::CreateBlendAttachment(PipelineInfo.EnableBlending);

        ColorBlendingState.srcColorBlendFactor = (VkBlendFactor)Info.SrcColorFactor;
        ColorBlendingState.dstColorBlendFactor = (VkBlendFactor)Info.DstColorFactor;
        ColorBlendingState.colorBlendOp = (VkBlendOp)Info.ColorOperation;

        ColorBlendingState.srcAlphaBlendFactor = (VkBlendFactor)Info.SrcAlphaFactor;
        ColorBlendingState.dstAlphaBlendFactor = (VkBlendFactor)Info.DstAlphaFactor;
        ColorBlendingState.alphaBlendOp = (VkBlendOp)Info.AlphaOperation;
    }

    /*
     * Color blending info
     * Currently doesn't support logic blending
     */
    VkPipelineColorBlendStateCreateInfo ColorBlendStageInfo = VKTool::CreateColorBlendStageInfo(
        false,
        VK_LOGIC_OP_COPY,
        ColorBlendingAttachments);
    
    /*
     * Depth stencil
     */
    VkPipelineDepthStencilStateCreateInfo DepthStencilStageInfo = VKTool::CreateDepthStencilStageInfo(
        PipelineInfo.EnableDepth,
        PipelineInfo.EnableDepthWrite,
        PipelineInfo.EnableStencil,
        (VkCompareOp)PipelineInfo.DepthCompareOperation);
    
    DepthStencilStageInfo.front = VKTool::CreateStencilOp(
        (VkCompareOp)PipelineInfo.StencilCompareOperation,
        PipelineInfo.StencilCompareMask,
        PipelineInfo.StencilWriteMask,
        PipelineInfo.StencilWriteValue,
        (VkStencilOp)PipelineInfo.OnPass,
        (VkStencilOp)PipelineInfo.OnFail,
        (VkStencilOp)PipelineInfo.OnDepthFail);
    
    DepthStencilStageInfo.back = DepthStencilStageInfo.front;

    /*
     * Dynamic stage, viewport and scissor
     */
    VkDynamicState DynamicStates[] = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};
    VkPipelineDynamicStateCreateInfo DynamicStateInfo {};
    DynamicStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    DynamicStateInfo.dynamicStateCount = 2;
    DynamicStateInfo.pDynamicStates = DynamicStates;
    
    VkGraphicsPipelineCreateInfo GraphicsPipelineInfo {};
    GraphicsPipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    GraphicsPipelineInfo.layout = PipelineLayout;
    GraphicsPipelineInfo.renderPass = VKPass->RenderPass;
    GraphicsPipelineInfo.subpass = Subpass;
    GraphicsPipelineInfo.stageCount = (uint32_t)ShaderStages.size();
    GraphicsPipelineInfo.pStages = ShaderStages.data();
    /*
     * No vertex input for none assemble pipeline
     */
    GraphicsPipelineInfo.pVertexInputState = VK_NULL_HANDLE;
    GraphicsPipelineInfo.pInputAssemblyState = VK_NULL_HANDLE;
    GraphicsPipelineInfo.pViewportState = &ViewportScissorInfo;
    GraphicsPipelineInfo.pRasterizationState = &RasterizationStageInfo;
    GraphicsPipelineInfo.pMultisampleState = &MultisampleInfo;
    GraphicsPipelineInfo.pColorBlendState = &ColorBlendStageInfo;
    GraphicsPipelineInfo.pDepthStencilState = &DepthStencilStageInfo;
    GraphicsPipelineInfo.pDynamicState = &DynamicStateInfo;
    GraphicsPipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

    VkPipeline GraphicsPipeline = VK_NULL_HANDLE;

    /*
     * No cache currently, may create cache later
     * TODO: use pipeline cache
     */
    VkResult Result = vkCreateGraphicsPipelines(
        VulkanContext->Device,
        nullptr,
        1,
        &GraphicsPipelineInfo,
        nullptr,
        &GraphicsPipeline);

    if(Result != VK_SUCCESS)
    {
        return nullptr;
    }

    return new VulkanRHIPipeline{{}, GraphicsPipeline, PipelineLayout};
}

RHIPipeline* VulkanRHI::CreateComputePipeline(const std::vector<RHIDescriptorLayout*>& Descriptors,
    RHIShaderModule* ShaderModule)
{
    std::vector<VkDescriptorSetLayout> Layouts;

    for(auto Descriptor : Descriptors)
    {
        RHI_2_VK_CHECKED_NULL(VulkanRHIDescriptorLayout, Descriptor, VKLayout)
        Layouts.push_back(VKLayout->Layout);
    }
    
    VkPipelineLayout PipelineLayout = VKTool::CreatePipelineLayout(VulkanContext, Layouts);
    if(PipelineLayout == VK_NULL_HANDLE)
    {
        return nullptr;
    }

    
    RHI_2_VK_CHECKED_NULL(VulkanRHIShaderModule, ShaderModule, VKShader)
    
    VkComputePipelineCreateInfo ComputePipelineInfo {};
    ComputePipelineInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
    ComputePipelineInfo.layout = PipelineLayout;
    ComputePipelineInfo.stage = VKTool::CreateShaderStageInfo(VKShader->ShaderModule,(VkShaderStageFlagBits)ShaderModule->ShaderType);;

    /*
     *TODO: Compute pipeline cache
     */
    VkPipeline ComputePipeline = VK_NULL_HANDLE;
    VkResult Result = vkCreateComputePipelines(VulkanContext->Device, nullptr, 1, &ComputePipelineInfo, nullptr, &ComputePipeline);
    if(Result != VK_SUCCESS)
    {
        return nullptr;
    }

    return new VulkanRHIPipeline{{}, ComputePipeline};
}

RHIFrameBuffer* VulkanRHI::CreateFrameBuffer(RHIRenderPass* ToPass, const std::vector<RHITextureView*>& ImageViews,
                                             uint32_t Width, uint32_t Height, uint32_t LayerCount)
{
    RHI_2_VK_CHECKED_NULL(VulkanRHIRenderPass, ToPass, ToVKPass)

    std::vector<VkImageView> VKImageViews(ImageViews.size());

    for(size_t i=0; i<ImageViews.size(); ++i)
    {
        RHI_2_VK_CHECKED_NULL(VulkanRHITextureView, ImageViews[i], VKTexImageView)
        VKImageViews[i] = VKTexImageView->ImageView;
    }
    
    VkFramebufferCreateInfo FrameBufferInfo {};
    FrameBufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    FrameBufferInfo.flags = 0;
    FrameBufferInfo.renderPass = ToVKPass->RenderPass;
    FrameBufferInfo.attachmentCount = (uint32_t)VKImageViews.size();
    FrameBufferInfo.pAttachments = VKImageViews.data();
    FrameBufferInfo.width = Width;
    FrameBufferInfo.height = Height;
    FrameBufferInfo.layers = LayerCount;

    VkFramebuffer FrameBuffer = VK_NULL_HANDLE;
    VkResult Result = vkCreateFramebuffer(VulkanContext->Device, &FrameBufferInfo, nullptr, &FrameBuffer);

    if(Result != VK_SUCCESS)
    {
        return nullptr;
    }
    
    return new VulkanRHIFrameBuffer{{}, FrameBuffer};
}

void VulkanRHI::StartRenderPass(RHICommandList* Phase, RHIRenderPass* Pass, RHIFrameBuffer* FrameBuffer,
    const std::vector<ClearColorInfo>& ColorClearInfos, RHIRect2D RenderArea)
{

    RHI_2_VK_CHECKED(VulkanRHICommandList, Phase, VKPhase)
    RHI_2_VK_CHECKED(VulkanRHIRenderPass, Pass, VKPass)
    RHI_2_VK_CHECKED(VulkanRHIFrameBuffer, FrameBuffer, VKFrameBuffer)
    
    std::vector<VkClearValue> ClearColors(ColorClearInfos.size());
    for(size_t i = 0; i<ColorClearInfos.size(); ++i)
    {
        if(ColorClearInfos[i].IsDepthStencil)
        {
            ClearColors[i].depthStencil = {ColorClearInfos[i].Depth, ColorClearInfos[i].Stencil};
        }
        else
        {
            ClearColors[i].color = {{
               ColorClearInfos[i].Color[0],
               ColorClearInfos[i].Color[1],
               ColorClearInfos[i].Color[2],
               ColorClearInfos[i].Color[3]}};
        }
    }
    
    VkRenderPassBeginInfo RenderPassBegin {};
    RenderPassBegin.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    RenderPassBegin.framebuffer = VKFrameBuffer->FrameBuffer;
    RenderPassBegin.renderPass = VKPass->RenderPass;
    RenderPassBegin.clearValueCount = (uint32_t)ClearColors.size();
    RenderPassBegin.pClearValues = ClearColors.data();
    RenderPassBegin.renderArea = {{RenderArea.OffsetX, RenderArea.OffsetY}, {RenderArea.Width, RenderArea.Height}};

    VkCommandBuffer StartRenderBuffer = VulkanContext->DrawCommandBuffers[VKPhase->BufferIndex];
    vkCmdBeginRenderPass(StartRenderBuffer, &RenderPassBegin, VK_SUBPASS_CONTENTS_INLINE);
}

void VulkanRHI::SetRenderViewport(RHICommandList* Phase, const RHIViewport& Viewport, uint32_t Index)
{
    RHI_2_VK_CHECKED(VulkanRHICommandList, Phase, VKPhase)
    VkViewport VKViewport {Viewport.XPos, Viewport.Height-Viewport.YPos, Viewport.Width, -Viewport.Height, 0.0, 1.0};
    vkCmdSetViewport(VulkanContext->DrawCommandBuffers[VKPhase->BufferIndex], Index, 1, &VKViewport);
}

void VulkanRHI::SetRenderScissor(RHICommandList* Phase, const RHIRect2D& Scissor, uint32_t Index)
{
    RHI_2_VK_CHECKED(VulkanRHICommandList, Phase, VKPhase)
    VkRect2D VKScissor = {{Scissor.OffsetX, Scissor.OffsetY}, {Scissor.Width, Scissor.Height}};
    vkCmdSetScissor(VulkanContext->DrawCommandBuffers[VKPhase->BufferIndex], Index, 1, &VKScissor);
}

void VulkanRHI::UseGraphicsPipeline(RHICommandList* Phase, RHIPipeline* GraphicsPipeline)
{
    RHI_2_VK_CHECKED(VulkanRHICommandList, Phase, VKPhase)
    RHI_2_VK_CHECKED(VulkanRHIPipeline, GraphicsPipeline, VKGraphicsPipeline)
    vkCmdBindPipeline(
        VulkanContext->DrawCommandBuffers[VKPhase->BufferIndex],
        VK_PIPELINE_BIND_POINT_GRAPHICS,
        VKGraphicsPipeline->Pipeline);
}

void VulkanRHI::UseComputePipeline(RHICommandList* Phase, RHIPipeline* ComputePipeline)
{
    RHI_2_VK_CHECKED(VulkanRHICommandList, Phase, VKPhase)
    RHI_2_VK_CHECKED(VulkanRHIPipeline, ComputePipeline, VKComputePipeline)
    vkCmdBindPipeline(
        VulkanContext->ComputeCommandBuffers[VKPhase->BufferIndex],
        VK_PIPELINE_BIND_POINT_COMPUTE,
        VKComputePipeline->Pipeline);
}

void VulkanRHI::SetDescriptorSet(RHICommandList* RenderingPhase, RHIPipeline* Pipeline, RHIDescriptorSet* Set,
    uint32_t BindIndex, const std::vector<uint32_t>& DynamicOffsets)
{
    RHI_2_VK_CHECKED(VulkanRHICommandList, RenderingPhase, VKPhase)
    RHI_2_VK_CHECKED(VulkanRHIPipeline, Pipeline, VKPipeline)
    RHI_2_VK_CHECKED(VulkanRHIDescriptorSet, Set, VKSet)
    VkCommandBuffer Buffer = VK_NULL_HANDLE;
    VkPipelineBindPoint Point = VK_PIPELINE_BIND_POINT_GRAPHICS;
    if(VKPhase->PhaseQueue == RenderingTaskQueue_Graphics)
    {
        Buffer = VulkanContext->DrawCommandBuffers[VKPhase->BufferIndex];
    }
    else
    {
        Buffer = VulkanContext->ComputeCommandBuffers[VKPhase->BufferIndex];
        Point = VK_PIPELINE_BIND_POINT_COMPUTE;
    }
    const uint32_t* OffsetPtr = nullptr;
    if(!DynamicOffsets.empty())
    {
        OffsetPtr = DynamicOffsets.data();
    }
    vkCmdBindDescriptorSets(Buffer, Point, VKPipeline->Layout, BindIndex, 1, &VKSet->Set, DynamicOffsets.size(), OffsetPtr);
}

void VulkanRHI::DrawMeshTask(RHICommandList* Phase, uint32_t WorkGroupX, uint32_t WorkGroupY, uint32_t WorkGroupZ)
{
    RHI_2_VK_CHECKED(VulkanRHICommandList, Phase, VKPhase)
    VulkanEXT.VKCmdDrawMeshTaskEXT(VulkanContext->DrawCommandBuffers[VKPhase->BufferIndex], WorkGroupX, WorkGroupY, WorkGroupZ);
    
}

void VulkanRHI::Dispatch(RHICommandList* Phase, uint32_t WorkGroupX, uint32_t WorkGropY, uint32_t WorkGroupZ)
{
    RHI_2_VK_CHECKED(VulkanRHICommandList, Phase, VKPhase)
    vkCmdDispatch(VulkanContext->ComputeCommandBuffers[VKPhase->BufferIndex], WorkGroupX, WorkGropY, WorkGroupZ);
}

void VulkanRHI::StartNextSubpass(RHICommandList* Phase)
{
    RHI_2_VK_CHECKED(VulkanRHICommandList, Phase, VKPhase)
    vkCmdNextSubpass(VulkanContext->DrawCommandBuffers[VKPhase->BufferIndex], VK_SUBPASS_CONTENTS_INLINE);
}

void VulkanRHI::EndRenderPass(RHICommandList* Phase)
{
    RHI_2_VK_CHECKED(VulkanRHICommandList, Phase, VKPhase)
    VkCommandBuffer EndBuffer = VulkanContext->DrawCommandBuffers[VKPhase->BufferIndex];
    vkCmdEndRenderPass(EndBuffer);
}

void VulkanRHI::BeginFrameRendering()
{
    while(true)
    {
        VkResult Result = vkAcquireNextImageKHR(
        VulkanContext->Device,
        VulkanContext->Swapchain,
        UINT64_MAX,
        VulkanContext->ImageRequireSemaphore,
        VK_NULL_HANDLE,
        &VulkanContext->CurrentFrameIndex);

        if(Result == VK_ERROR_OUT_OF_DATE_KHR)
        {
            SyncRunningCommands();
            RecreateSwapChain();
            SwapchainRecreateFunction(VulkanContext->Extend.width, VulkanContext->Extend.height);
        }
    
        else if(Result != VK_SUCCESS && Result != VK_SUBOPTIMAL_KHR)
        {
            LOG_ERROR_FUNCTION("Error when aquire swapchain image");
            assert(false);
        }
        else
        {
            break;
        }
    }
    
}

void VulkanRHI::EndFrameRendering()
{
    VkPresentInfoKHR PresentInfo {};
    PresentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    PresentInfo.waitSemaphoreCount = 1;
    PresentInfo.pWaitSemaphores = &VulkanContext->PresentSemaphore;
    PresentInfo.swapchainCount = 1;
    PresentInfo.pSwapchains = &VulkanContext->Swapchain;
    PresentInfo.pImageIndices = &VulkanContext->CurrentFrameIndex;

    VkResult Result = vkQueuePresentKHR(VulkanContext->PresentQueue, &PresentInfo);

    if(Result == VK_ERROR_OUT_OF_DATE_KHR || Result == VK_SUBOPTIMAL_KHR)
    {
        SyncRunningCommands();
        RecreateSwapChain();
        SwapchainRecreateFunction(VulkanContext->Extend.width, VulkanContext->Extend.height);
    }
    else if(Result != VK_SUCCESS)
    {
        LOG_ERROR_FUNCTION("Fail to submit rendering result");
        assert(false);
    }

   SyncRunningCommands();
    
}

void VulkanRHI::DestroyTexImageView(RHITextureView*& Destroyed)
{
    RHI_2_VK_CHECKED(VulkanRHITextureView, Destroyed, VKImageViewDestroyed)
    vkDestroyImageView(VulkanContext->Device, VKImageViewDestroyed->ImageView, nullptr);
    delete VKImageViewDestroyed;
}

void VulkanRHI::DestroyTexture(RHITexture*& Destroyed)
{
    RHI_2_VK_CHECKED(VulkanRHITexture, Destroyed, VKTextureDestroyed)
    DestroyTexImageView(VKTextureDestroyed->DefaultTextureView);
    
    vmaDestroyImage(VulkanContext->Allocator, VKTextureDestroyed->Image, VKTextureDestroyed->Allocation);
    
    delete VKTextureDestroyed;
}

void VulkanRHI::DestroyBuffer(RHIBuffer*& Destroyed)
{
    RHI_2_VK_CHECKED(VulkanRHIBuffer, Destroyed, VKBufferDestroyed)
    if(VKBufferDestroyed->MemoryUsage != BufferMemoryUsage_GPU_Only)
    {
        vmaUnmapMemory(VulkanContext->Allocator, VKBufferDestroyed->Allocation);
    }
    vmaDestroyBuffer(VulkanContext->Allocator, VKBufferDestroyed->BufferObject, VKBufferDestroyed->Allocation);
}


void VulkanRHI::DestroyFrameBuffer(RHIFrameBuffer*& Destroyed)
{
    RHI_2_VK_CHECKED(VulkanRHIFrameBuffer, Destroyed, VKFrameBufferDestroyed)
    vkDestroyFramebuffer(VulkanContext->Device, VKFrameBufferDestroyed->FrameBuffer, nullptr);
    delete VKFrameBufferDestroyed;
}

void VulkanRHI::DestroyPipeline(RHIPipeline*& Destroyed)
{
    RHI_2_VK_CHECKED(VulkanRHIPipeline, Destroyed, VKPipelineDestroyed)
    vkDestroyPipeline(VulkanContext->Device, VKPipelineDestroyed->Pipeline, nullptr);
    vkDestroyPipelineLayout(VulkanContext->Device, VKPipelineDestroyed->Layout, nullptr);
    delete Destroyed;
}

void VulkanRHI::DestroyRenderPass(RHIRenderPass*& Destroyed)
{
    RHI_2_VK_CHECKED(VulkanRHIRenderPass, Destroyed, VKPassDestroyed)
    vkDestroyRenderPass(VulkanContext->Device, VKPassDestroyed->RenderPass, nullptr);
    delete VKPassDestroyed;
}

void VulkanRHI::DestroyDescriptorLayout(RHIDescriptorLayout*& Destroyed)
{
    RHI_2_VK_CHECKED(VulkanRHIDescriptorLayout, Destroyed, VKLayoutDestroyed)
    vkDestroyDescriptorSetLayout(VulkanContext->Device, VKLayoutDestroyed->Layout, nullptr);
    delete VKLayoutDestroyed;
}

void VulkanRHI::DestroyDescriptorSet(RHIDescriptorSet*& Destroyed)
{
    RHI_2_VK_CHECKED(VulkanRHIDescriptorSet, Destroyed, VKSetDestroyed)
    vkFreeDescriptorSets(VulkanContext->Device, VulkanContext->DescriptorPool, 1, &VKSetDestroyed->Set);
    delete VKSetDestroyed;
}

void VulkanRHI::DestroySemaphoreView(RHISemaphoreView*& Destroyed)
{
    delete Destroyed;
}

void VulkanRHI::DestroySemaphore(RHISemaphore*& Destroyed)
{
    RHI_2_VK_CHECKED(VulkanRHISemaphore, Destroyed, VKSemaphoreDestroyed)
    vkDestroySemaphore(VulkanContext->Device, VKSemaphoreDestroyed->VulkanSemaphore, nullptr);
    delete Destroyed;
}

void VulkanRHI::DestroySampler(RHISampler*& Destroyed)
{
    RHI_2_VK_CHECKED(VulkanRHISampler, Destroyed, VKSamplerDestroyed)
    delete Destroyed;
}

void VulkanRHI::DestroyShader(RHIShaderModule*& Destroyed)
{
    RHI_2_VK_CHECKED(VulkanRHIShaderModule, Destroyed, VKDestroyedShader)
    vkDestroyShaderModule(VulkanContext->Device, VKDestroyedShader->ShaderModule, nullptr);
    delete VKDestroyedShader;
}

RHIViewport VulkanRHI::GetDefaultViewport() const
{
    return RHIViewport{
        VulkanContext->DefaultViewport.x,
        VulkanContext->DefaultViewport.y,
        VulkanContext->DefaultViewport.width,
        -VulkanContext->DefaultViewport.height};
}

void VulkanRHI::SetDefaultViewport(const RHIViewport& NewViewport)
{
    VulkanContext->DefaultViewport.x = NewViewport.XPos;
    VulkanContext->DefaultViewport.y = NewViewport.YPos;
    VulkanContext->DefaultViewport.width = NewViewport.Width;
    VulkanContext->DefaultViewport.height = -NewViewport.Height;
}

RHIRect2D VulkanRHI::GetDefaultScissor() const
{
    return RHIRect2D{
        VulkanContext->DefaultScissor.extent.width,
        VulkanContext->DefaultScissor.extent.height,
        VulkanContext->DefaultScissor.offset.x,
        VulkanContext->DefaultScissor.offset.y};
}

void VulkanRHI::SetDefaultScissor(const RHIRect2D& NewScissor)
{
    VulkanContext->DefaultScissor.offset.x = NewScissor.OffsetX;
    VulkanContext->DefaultScissor.offset.y = NewScissor.OffsetY;
    VulkanContext->DefaultScissor.extent.width = NewScissor.Width;
    VulkanContext->DefaultScissor.extent.height = NewScissor.Height;
}

std::string VulkanRHI::GetRHIName() const
{
    return "VulkanRHI";
}

std::string VulkanRHI::GetShaderNameSuffix() const
{
    return "spv";
}

VkCommandBuffer VulkanRHI::CreateSingletTimeCommandBuffer()
{
    VkCommandBufferAllocateInfo CommandBufferInfo {};
    CommandBufferInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    CommandBufferInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    CommandBufferInfo.commandPool = VulkanContext->SingleTimeCommandPool;
    CommandBufferInfo.commandBufferCount = 1;

    VkCommandBuffer CommandBuffer = VK_NULL_HANDLE;

    VkResult Result = vkAllocateCommandBuffers(VulkanContext->Device, &CommandBufferInfo, &CommandBuffer);

    if(Result == VK_SUCCESS)
    {
        VkCommandBufferBeginInfo BeginInfo {};
        BeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        BeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
        vkBeginCommandBuffer(CommandBuffer, &BeginInfo);
    }
    else
    {
        return VK_NULL_HANDLE;
    }
    
    return CommandBuffer;
}

void VulkanRHI::EndSingleTimeCommandBuffer(VkCommandBuffer Cmd)
{
    vkEndCommandBuffer(Cmd);
    VkSubmitInfo SubmitInfo {};
    SubmitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    SubmitInfo.commandBufferCount = 1;
    SubmitInfo.pCommandBuffers = &Cmd;
    SubmitInfo.waitSemaphoreCount = 0;
    SubmitInfo.pWaitSemaphores = nullptr;
    SubmitInfo.pWaitDstStageMask = nullptr;
    SubmitInfo.signalSemaphoreCount = 0;
    SubmitInfo.pSignalSemaphores = nullptr;
    vkQueueSubmit(VulkanContext->GraphicsQueue, 1, &SubmitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(VulkanContext->GraphicsQueue);
}

void VulkanRHI::EndStageBuffer(VkBuffer StageBuffer, VkDeviceMemory BufferMemory)
{
    vkDestroyBuffer(VulkanContext->Device, StageBuffer, nullptr);
    vkFreeMemory(VulkanContext->Device, BufferMemory, nullptr);
}