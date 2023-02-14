#pragma once

#include <unordered_map>
#include <vector>

#include "OpenRenderRuntime/Core/VulkanRHI/VulkanRHIRenderingPhase.h"
#include "OpenRenderRuntime/Core/VulkanRHI/VulkanRHIContext.h"
#include "OpenRenderRuntime/Core/RHI/RHI.h"
#include "OpenRenderRuntime/Core/RHI/RHIBuffer.h"

#include "OpenRenderRuntime/Core/VulkanRHI/VulkanRHIBuffer.h"

class VulkanRHI : public RHI
{
    VulkanExtFunctionSet VulkanEXT = {};

    std::vector<const char*> LayerRequires = {}; 

    std::vector<const char *> InstanceExtensionRequires = {VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME};
    std::vector<const char *> DeviceExtensionRequires {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME,
        VK_EXT_MESH_SHADER_EXTENSION_NAME,
        VK_KHR_SPIRV_1_4_EXTENSION_NAME,
        VK_KHR_SHADER_FLOAT_CONTROLS_EXTENSION_NAME};
    
    VulkanRHIContext* VulkanContext = nullptr;

    //Command buffer status marking
    VulkanCmdBufferStatusStruct DrawCommandsStatus {};
    VulkanCmdBufferStatusStruct ComputeCommandsStatus {};
    
    //Command buffer
    std::vector<VulkanPhaseSubmitInfo> DeferredSubmitInfo {};

    //Sampler map
    std::unordered_map<uint64_t, VkSampler> SamplerTable {};
    
    constexpr static VkFormat FormatTransfer[] = {
        VK_FORMAT_R8G8B8_UNORM,
        VK_FORMAT_R8G8B8_SRGB,
        VK_FORMAT_R32G32B32_SFLOAT,
        VK_FORMAT_R8G8B8A8_UNORM,
        VK_FORMAT_R8G8B8A8_SRGB,
        VK_FORMAT_R32G32B32A32_SFLOAT,
        VK_FORMAT_R32_SFLOAT,
        VK_FORMAT_D32_SFLOAT,
        VK_FORMAT_B8G8R8A8_UNORM,
        VK_FORMAT_B8G8R8A8_SRGB,
        VK_FORMAT_R16G16B16A16_SFLOAT,
        VK_FORMAT_A2B10G10R10_UNORM_PACK32};

    constexpr static VkImageUsageFlags AttachmentUsageTransfer[] = {
        VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT,
        VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT,
        VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT,
        VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT ,
        VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT,
        VK_IMAGE_USAGE_STORAGE_BIT};

    constexpr static VkImageUsageFlagBits UsageBitTransfer[] = {
        VK_IMAGE_USAGE_SAMPLED_BIT,
        VK_IMAGE_USAGE_STORAGE_BIT,
        VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
        VK_IMAGE_USAGE_TRANSFER_DST_BIT,
        VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT,
        VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT};

    constexpr static VkImageAspectFlags AttachmentAspectTransfer[] {
        VK_IMAGE_ASPECT_COLOR_BIT,
        VK_IMAGE_ASPECT_DEPTH_BIT,
        VK_IMAGE_ASPECT_COLOR_BIT,
        VK_IMAGE_ASPECT_COLOR_BIT,
        VK_IMAGE_ASPECT_STENCIL_BIT | VK_IMAGE_ASPECT_DEPTH_BIT,
        VK_IMAGE_ASPECT_COLOR_BIT
    };

    constexpr static VmaMemoryUsage BufferMemoryUsageTransfer[] = {
        VMA_MEMORY_USAGE_GPU_ONLY,
        VMA_MEMORY_USAGE_CPU_COPY,
        VMA_MEMORY_USAGE_CPU_TO_GPU,
        VMA_MEMORY_USAGE_CPU_COPY};

    constexpr static VkBufferUsageFlagBits BufferUsageTransfer[] = {
        VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
        VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
        VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
        VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT};

    constexpr static VkSamplerAddressMode WrapTransfer[] = {
        VK_SAMPLER_ADDRESS_MODE_REPEAT,
        VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT,
        VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
        VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER};

    constexpr static VkFilter FilterTypeTransfer[] = {
        VK_FILTER_NEAREST,
        VK_FILTER_LINEAR,
        VK_FILTER_CUBIC_EXT};

    constexpr static VkSamplerMipmapMode MipmapSampleTransfer[] = {
        VK_SAMPLER_MIPMAP_MODE_NEAREST,
        VK_SAMPLER_MIPMAP_MODE_LINEAR};

    constexpr static VkPipelineStageFlagBits PipelineStageTransfer[] = {
        VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
        VK_PIPELINE_STAGE_VERTEX_SHADER_BIT,
        VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
        VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
        VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
        VK_PIPELINE_STAGE_TASK_SHADER_BIT_EXT,
        VK_PIPELINE_STAGE_MESH_SHADER_BIT_EXT,
        VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
        VK_PIPELINE_STAGE_TRANSFER_BIT,
        VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
        VK_PIPELINE_STAGE_HOST_BIT,
        VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT,
        VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
        VK_PIPELINE_STAGE_DRAW_INDIRECT_BIT};

    constexpr static uint32_t FormatSizeTransfer[] = {3, 3, 12, 4, 4, 16, 4, 4, 4, 4, 8, 4};

    constexpr static VkDescriptorType DescriptorTypeTransfer[] = {
        VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
        VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
        VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
        VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
        VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC,
        VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC,
        VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT,
        VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
        VK_DESCRIPTOR_TYPE_SAMPLER};
    
    void InitializeVulkan();
    void CreateVulkanInstance();
    void LoadExtensionFunctions();
    void SetupMessengerUtil();
    void CreateWindowSurface();
    void PickPhysicalDevice();
    void CreateLogicalDevice();
    void LoadDeviceExtensionFunctions();
    void CreateSwapchain();
    void CreateSwapchainImageView();
    void CreateDefaultAttachments();
    void CreateVmaAllocator();
    void CreateCommandPool();
    void CreateCommandBuffers();
    void CreateSyncPrimitives();
    void CreateDescriptorPool();

    void RecreateSwapChain();

    void DestroySwapchainObjects();
    void DestroyVulkanObjects();

    VkPipelineStageFlags TransferPipelineStage(PipelineStage Stages);

    /*
     * For host writable memory, copy data directly
     * For gpu only or host only read data, use staging buffer
     */ 
    void SetBufferDataDirect(VulkanRHIBuffer* Buffer, const void* Data, size_t Size, size_t Offset);
    void SetBufferDataStaging(VulkanRHIBuffer* Buffer, const void* Data, size_t Size, size_t Offset);

    void ClearRunningCommandBuffer(VulkanCmdBufferStatusStruct& Struct);
    void SyncRunningCommands();
    
    int TryGetValidCommandIndex(VulkanCmdBufferStatusStruct& Struct);
    void TryEndCommandIndex(VulkanCmdBufferStatusStruct& Struct, int Index);

    /*
     * 0-5 UVW Wrap 6-9 Min Mag Filter 10 Mipmap Filter 11-25 Mipmap level 26-31 Anisotropy
     * 32 - 63 border color
     */
    uint64_t EncodingSampler(uint32_t MipmapLevel, const TextureSamplerCreateStruct& SampleInfo, uint32_t Anisotropy);
    VkSampler GetSampler(uint32_t MipmapLevel,const TextureSamplerCreateStruct& SampleInfo,uint32_t Anisotropy);
    
    
    void DestroyAllSampler();
public:

    void PreInit() override;

    void Init() override;

    void PreTerminate() override;    
    void Terminate() override;

    void SetupContext() override;

    RHIShaderModule* CreateVertexShader(const void* ShaderSource, size_t Size) override;
    RHIShaderModule* CreateFragmentShader(const void* ShaderSource, size_t Size) override;
    RHIShaderModule* CreateMeshShader(const void* ShaderSource, size_t Size) override;
    RHIShaderModule* CreateTaskShader(const void* ShaderSource, size_t Size) override;
    RHIShaderModule* CreateComputeShader(const void* ShaderSource, size_t Size) override;

    RHIBuffer* CreateStorageBuffer(size_t Size, BufferMemoryUsage Usage, bool Share = false) override;
    RHIBuffer* CreateUniformBuffer(size_t Size) override;
    RHIBuffer* CreateVertexBuffer(size_t Size) override;
    RHIBuffer* CreateIndexBuffer(size_t Size) override;
    RHIBuffer* CreateAnyBuffer(size_t Size, BufferTypeFlag BufferType, BufferMemoryUsage Usage, bool Share) override;

    void SetBufferData(RHIBuffer* Buffer, const void* Data, size_t Size, size_t Offset) override;
    
    RHITexture* CreateTexture2DManualMipmap(
        uint32_t Width,
        uint32_t Height,
        TexturePixelFormat Format,
        const std::vector<std::vector<void*>>& Tex,
	    TextureType TextureCreateType,
        ParamUsage Usage,
        uint32_t MipmapLevelCount,
        uint32_t Anisotropy,
        uint32_t LayerCount,
        const TextureSamplerCreateStruct& SamplerInfo) override;
    RHITexture* CreateTexture2DAutoMipmap(
        uint32_t Width,
        uint32_t Height,
        TexturePixelFormat Format,
        const std::vector<void*>& Tex,
	    TextureType TextureCreateType,
        ParamUsage Usage,
        uint32_t MipmapLevelCount,
        uint32_t LayerCount,
        uint32_t Anisotropy,
        const TextureSamplerCreateStruct& SamplerInfo) override;

    RHIRenderImageAttachment* CreateRenderAttachment(
        uint32_t Width,
        uint32_t Height,
        TexturePixelFormat Format,
        RenderImageAttachmentType AttachmentType,
        ImageExtraUsage ExtraUsage,
        uint32_t LayerCount,
        uint32_t MipmapCount,
        bool Share) override;
    RHITexImageView* CreateImageViewFromImageAttachment(
        RHIRenderImageAttachment* ImageAttachment,
        uint32_t LayerCount,
        uint32_t MipmapCount,
        uint32_t Layer,
        uint32_t MipmapLevel,
        TextureType ImageViewType) override;
    RHITexture* CreateTextureFromImageAttachment(
        RHIRenderImageAttachment* ImageAttachment,
        uint32_t LayerCount,
        uint32_t MipmapCount,
        uint32_t Layer,
        uint32_t MipmapLevel,
        TextureType ImageViewType,
        uint32_t Anisotropy,
        const TextureSamplerCreateStruct& SampleInfo) override;
    
    uint32_t GetSwapchainImageCount() const override;
    TexturePixelFormat GetSwapchainImageFormat() const override;
    uint32_t GetSwapchainExtendWidth() override;
    uint32_t GetSwapchainExtendHeight() override;
    RHIRenderImageAttachment& GetSwapchainRenderAttachment(uint32_t Index) override;

    uint32_t GetCurrentSwapchainImageIndex() override;
    
    RHIRenderingPhase* GetRenderPhase(RenderingTaskQueue PhaseQueue) override;
    void SubmitRenderingPhase(
        RHIRenderingPhase*& SubmittedPhase,
        const std::vector<RHISemaphoreView*>& WaitSemaphore,
        const std::vector<RHISemaphore*>& SignalSemaphore) override;

    RHISemaphore* GetRenderStartSemaphore() override;
    RHISemaphore* GetRenderEndSemaphore() override;
    RHISemaphore* CreateSemaphore() override;
    RHISemaphoreView* CreateSemaphoreView(RHISemaphore* Semaphore, PipelineStage WaitStage) override;

    RHIRenderPass* CreateRenderPass(const std::vector<AttachmentDescription>& Attachments, const std::vector<SubpassInfo>& SubpassInfos) override;

    RHIDescriptorLayout* CreateDescriptorLayout(const std::vector<DescriptorBindingInfo>& BindingInfos) override;
    RHIDescriptorSet* CreateDescriptorSet(RHIDescriptorLayout* RHILayout) override;
    void WriteDescriptorSetMulti(RHIDescriptorSet* WriteSet, const std::vector<TextureWriteInfo>& Textures, const std::vector<ImageWriteInfo>& Images, const std::vector<BufferWriteInfo>& Buffers) override;

    RHIPipeline* CreateGraphicsPipeline(
        RHIRenderPass* Pass,
        uint32_t Subpass,
        const std::vector<RHIDescriptorLayout*>& Descriptors,
        std::vector<RHIShaderModule*> ShaderModules,
        const GraphicsPipelineFixedFunctionInfo& PipelineInfo) override;
    RHIPipeline* CreateComputePipeline(const std::vector<RHIDescriptorLayout*>& Descriptors, RHIShaderModule* ShaderModule) override;

    RHIFrameBuffer* CreateFrameBuffer(
        RHIRenderPass* ToPass,
        const std::vector<RHITexImageView*>& ImageViews,
        uint32_t Width,
        uint32_t Height,
        uint32_t LayerCount) override;

    void StartRenderPass(
        RHIRenderingPhase* Phase,
        RHIRenderPass* Pass,
        RHIFrameBuffer* FrameBuffer,
        const std::vector<ClearColorInfo>& ColorClearInfos,
        RHIRect2D RenderArea) override;
    void SetRenderViewport(RHIRenderingPhase* Phase, const RHIViewport& Viewport, uint32_t Index) override;
    void SetRenderScissor(RHIRenderingPhase* Phase, const RHIRect2D& Scissor, uint32_t Index) override;
    void UseGraphicsPipeline(RHIRenderingPhase* Phase, RHIPipeline* GraphicsPipeline) override;
    void UseComputePipeline(RHIRenderingPhase* Phase, RHIPipeline* ComputePipeline) override;
    void SetDescriptorSet(RHIRenderingPhase* RenderingPhase, RHIPipeline* Pipeline, RHIDescriptorSet* Set, uint32_t BindIndex, const std::vector<uint32_t>& DynamicOffsets) override;
    void DrawMeshTask(RHIRenderingPhase* Phase, uint32_t WorkGroupX, uint32_t WorkGroupY, uint32_t WorkGroupZ) override;
    void StartNextSubpass(RHIRenderingPhase* Phase) override;
    void EndRenderPass(RHIRenderingPhase* Phase) override;

    void TranslateImageLayout(
        RHIRenderingPhase* RenderPhase,
        RHIRenderImageAttachment* ToAttachment,
        TexImageLayout Src,
        TexImageLayout Dst,
        PipelineStage WaitSrcStage,
        PipelineStage DstStageWait,
        uint32_t LayerCount,
        uint32_t MipmapCount,
        uint32_t LayerBias,
        uint32_t MipmapBias) override;
    void TranslateImageLayoutAccess(
        RHIRenderingPhase* RenderPhase,
        RHIRenderImageAttachment* ToAttachment,
        TexImageLayout Src,
        TexImageLayout Dst,
        PipelineStage WaitSrcStage,
        PipelineStage DstStageWait,
        AccessMask SrcAccessMask,
        AccessMask DstAccessMask,
        uint32_t LayerCount,
        uint32_t MipmapCount,
        uint32_t LayerBias,
        uint32_t MipmapBias) override;
    
    void BeginFrameRendering() override;
    void EndFrameRendering() override;

    void DestroyTexImageView(RHITexImageView*& Destroyed) override;
    void DestroyRenderAttachment(RHIRenderImageAttachment*& Destroyed) override;
    void DestroyTexture(RHITexture*& Destroyed) override;
    void DestroyBuffer(RHIBuffer*& Destroyed) override;
    void DestroyFrameBuffer(RHIFrameBuffer*& Destroyed) override;
    void DestroyPipeline(RHIPipeline*& Destroyed) override;
    void DestroyRenderPass(RHIRenderPass*& Destroyed) override;
    void DestroyDescriptorLayout(RHIDescriptorLayout* Destroyed) override;
    void DestroyDescriptorSet(RHIDescriptorSet* Destroyed) override;
    void DestroySemaphoreView(RHISemaphoreView* Destroyed) override;
    void DestroySemaphore(RHISemaphore* Destroyed) override;
    
    void DestroyShader(RHIShaderModule*& Destroyed) override;
    
    RHIViewport GetDefaultViewport() const override;
    void SetDefaultViewport(const RHIViewport& NewViewport) override;
    RHIRect2D GetDefaultScissor() const override;
    void SetDefaultScissor(const RHIRect2D& NewScissor) override;

    std::string GetRHIName() const override;
    std::string GetShaderNameSuffix() const override;
    
    // VulkanRHI exclusive functions
    VkCommandBuffer CreateSingletTimeCommandBuffer();
    void EndSingleTimeCommandBuffer(VkCommandBuffer Cmd);
    void EndStageBuffer(VkBuffer StageBuffer, VkDeviceMemory BufferMemory);

};
