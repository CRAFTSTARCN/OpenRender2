#pragma once

#include <functional>
#include <string>
#include <vector>

#include "OpenRenderRuntime/Core/RHI/RHIFrameBuffer.h"
#include "OpenRenderRuntime/Core/RHI/RHISemaphoreView.h"
#include "OpenRenderRuntime/Core/RHI/RHIPipeline.h"
#include "OpenRenderRuntime/Core/RHI/RHIScreen.h"
#include "OpenRenderRuntime/Core/RHI/RHIDescriptor.h"
#include "OpenRenderRuntime/Core/RHI/RHIRenderPass.h"
#include "OpenRenderRuntime/Core/RHI/RHIBuffer.h"
#include "OpenRenderRuntime/Core/RHI/RHIContext.h"
#include "OpenRenderRuntime/Core/RHI/RHIRenderingPhase.h"
#include "OpenRenderRuntime/Core/RHI/RHIShaderModule.h"
#include "OpenRenderRuntime/Core/RHI/RHITexture.h"
#include "OpenRenderRuntime/Core/RHI/RenderWindowProxy.h"
#include "OpenRenderRuntime/Core/RHI/RHIRenderImageAttachment.h"
#include "OpenRenderRuntime/Core/RHI/RHISemaphore.h"

class RHI
{

protected:
    
    GLFWwindow* WindowPtr;
    RHIContext* Context;

	//Default render attachment
	std::vector<RHIRenderImageAttachment*> DefaultAttachments {};

	//Resize function
	std::function<void(uint32_t NewWidth, uint32_t NewHeight)> SwapchainRecreateFunction = nullptr;
public:

    RHI();
    virtual ~RHI();

    RHI(const RHI&) = delete;
    RHI(RHI&&) = delete;
    RHI& operator=(const RHI&) = delete;
    RHI& operator=(RHI&&) = delete;
    
    virtual void PreInit() = 0;
    virtual void Init() = 0;

    virtual RenderWindowProxy& GetWindowProxy();

	virtual void PreTerminate() = 0;
    virtual void Terminate() = 0;

    virtual void SetupContext() = 0;

    /*
     * Shader creation api
     * From shading language source code or IL code
     */
    virtual RHIShaderModule* CreateVertexShader(const void* ShaderSource, size_t Size) = 0;
    virtual RHIShaderModule* CreateFragmentShader(const void* ShaderSource, size_t Size) = 0;
    virtual RHIShaderModule* CreateMeshShader(const void* ShaderSource, size_t Size) = 0;
    virtual RHIShaderModule* CreateTaskShader(const void* ShaderSource, size_t Size) = 0;
    virtual RHIShaderModule* CreateComputeShader(const void* ShaderSource, size_t Size) = 0;

    /*
     * Buffer creation
     * Uniform buffer accept direct data from CPU
     * Vertex and index buffer can only accept data from staging buffer
     */
    virtual RHIBuffer* CreateStorageBuffer(size_t Size, BufferMemoryUsage Usage, bool Share) = 0;
    virtual RHIBuffer* CreateUniformBuffer(size_t Size) = 0;
    virtual RHIBuffer* CreateVertexBuffer(size_t Size) = 0;
    virtual RHIBuffer* CreateIndexBuffer(size_t Size) = 0;
    /*
     * Any buffer (usage and memory usage) creation
     * For example, if you want to create a vertex buffer which data will change each frame, use this api
     */
    virtual RHIBuffer* CreateAnyBuffer(size_t Size, BufferTypeFlag BufferType, BufferMemoryUsage Usage, bool Share) = 0;

	virtual void SetBufferData(RHIBuffer* Buffer, const void* Data, size_t Size, size_t Offset) = 0;

	/*
	 * Manual mipmap tex vector: [mipmap1[layer1, layer2, ...] mipmap2[layer1, layer2, ...], ...]
	 * Texture mipmap, layer and type check should be granted by caller,
	 * no check will execute by RHI,
	 * wrong param may cause api fail, nullptr will returned
	 */
    virtual RHITexture* CreateTexture2DManualMipmap(
	    uint32_t Width,
	    uint32_t Height,
	    TexturePixelFormat Format,
	    const std::vector<std::vector<void*>>& Tex,
	    TextureType TextureCreateType,
	    ParamUsage Usage = ParamUsageBit_Fragment,
	    uint32_t MipmapLevelCount = 1,
	    uint32_t Anisotropy = 0,
	    uint32_t LayerCount = 1,
	    const TextureSamplerCreateStruct& SamplerInfo = {}) = 0;
	/*
	 * If mipmap is 0, mipmap will be set automatically
	 */
	virtual RHITexture* CreateTexture2DAutoMipmap(
		uint32_t Width,
		uint32_t Height,
		TexturePixelFormat Format,
		const std::vector<void*>& Tex,
	    TextureType TextureCreateType,
		ParamUsage Usage = ParamUsageBit_Fragment,
		uint32_t MipmapLevelCount = 1,
		uint32_t LayerCount = 1,
		uint32_t Anisotropy = 0,
		const TextureSamplerCreateStruct& SampleInfo = {}) = 0;

	/*
	 * Generate render attachment, the attachment type will first give a basic usage
	 * Default view will created, with first level of mipmap and all layer
	 */
	virtual RHIRenderImageAttachment* CreateRenderAttachment(
		uint32_t Width,
		uint32_t Height,
		TexturePixelFormat Format,
		RenderImageAttachmentType AttachmentType,
		ImageExtraUsage ExtraUsage,
		uint32_t LayerCount = 1,
		uint32_t MipmapCount = 1,
		bool Share = false) = 0;
	virtual RHITexImageView* CreateImageViewFromImageAttachment(
		RHIRenderImageAttachment* ImageAttachment,
		uint32_t LayerCount = 1,
		uint32_t MipmapCount = 1,
		uint32_t Layer = 0,
		uint32_t MipmapLevel = 0,
		TextureType ImageViewType = TextureType_2D) = 0;
	virtual RHITexture* CreateTextureFromImageAttachment(
		RHIRenderImageAttachment* ImageAttachment,
		uint32_t LayerCount = 1,
		uint32_t MipmapCount = 1,
		uint32_t Layer = 0,
		uint32_t MipmapLevel = 0,
		TextureType TextureCreateType = TextureType_2D,
		uint32_t Anisotropy = 0,
		const TextureSamplerCreateStruct& SampleInfo = {}) = 0;
	
	virtual uint32_t GetSwapchainImageCount() const = 0;
	virtual TexturePixelFormat GetSwapchainImageFormat() const = 0;
	virtual uint32_t GetSwapchainExtendWidth() = 0;
	virtual uint32_t GetSwapchainExtendHeight() = 0;
	/*
	 * All image and view of swapchain image should create and managed by RHI
	 */
	virtual RHIRenderImageAttachment& GetSwapchainRenderAttachment(uint32_t Index) = 0;

	virtual uint32_t GetCurrentSwapchainImageIndex() = 0;

	/*
	 * Render phase getting and submitting should thread safe
	 */
    virtual RHIRenderingPhase* GetRenderPhase(RenderingTaskQueue PhaseQueue) = 0;
	/*
	 * Submitted phase will deleted
	 */
	virtual void SubmitRenderingPhase(
		RHIRenderingPhase*& SubmittedPhase,
		const std::vector<RHISemaphoreView*>& WaitSemaphore,
		const std::vector<RHISemaphore*>& SignalSemaphore) = 0;

	/*
	 * This two semaphore should built-in with rhi
	 */
	virtual RHISemaphore* GetRenderStartSemaphore() = 0;
	virtual RHISemaphore* GetRenderEndSemaphore() = 0;
	virtual RHISemaphore* CreateSemaphore() = 0;
	virtual RHISemaphoreView* CreateSemaphoreView(RHISemaphore* Semaphore, PipelineStage WaitStage) = 0;
	
	virtual RHIRenderPass* CreateRenderPass(const std::vector<AttachmentDescription>& Attachments, const std::vector<SubpassInfo>& SubpassInfos) = 0;

	virtual RHIDescriptorLayout* CreateDescriptorLayout(const std::vector<DescriptorBindingInfo>& BindingInfos) = 0;
	virtual RHIDescriptorSet* CreateDescriptorSet(RHIDescriptorLayout* RHILayout) = 0;
	virtual void WriteDescriptorSetMulti(
		RHIDescriptorSet* WriteSet,
		const std::vector<TextureWriteInfo>& Textures,
		const std::vector<ImageWriteInfo>& Images,
		const std::vector<BufferWriteInfo>& Buffers) = 0;
	
	virtual RHIPipeline* CreateGraphicsPipeline(
		RHIRenderPass* Pass,
		uint32_t Subpass,
		const std::vector<RHIDescriptorLayout*>& Descriptors,
		std::vector<RHIShaderModule*> ShaderModules, const GraphicsPipelineFixedFunctionInfo& PipelineInfo) = 0;
	virtual RHIPipeline* CreateComputePipeline(const std::vector<RHIDescriptorLayout*>& Descriptors, RHIShaderModule* ShaderModule) = 0;

	virtual RHIFrameBuffer* CreateFrameBuffer(
		RHIRenderPass* ToPass,
		const std::vector<RHITexImageView*>& ImageViews,
		uint32_t Width,
		uint32_t Height,
		uint32_t LayerCount) = 0;
	
	/*
	 * Render orders
	 * Notice, order recorded to different render phase is THREAD SAFE!
	 */
	/*
	 * For StartRenderPass order, rendering pass, frame buffer and current subpass info should stored in phase
	 * So no subpass index info or should pass to next subpass or end pass
	 */
	virtual void StartRenderPass(
		RHIRenderingPhase* Phase,
		RHIRenderPass* Pass,
		RHIFrameBuffer* FrameBuffer,
		const std::vector<ClearColorInfo>& ColorClearInfos,
		RHIRect2D RenderArea) = 0;
	virtual void SetRenderViewport(RHIRenderingPhase* Phase, const RHIViewport& Viewport, uint32_t Index) = 0;
	virtual void SetRenderScissor(RHIRenderingPhase* Phase, const RHIRect2D& Scissor, uint32_t Index) = 0;
	virtual void UseGraphicsPipeline(RHIRenderingPhase* Phase, RHIPipeline* GraphicsPipeline) = 0;
	virtual void UseComputePipeline(RHIRenderingPhase* Phase, RHIPipeline* ComputePipeline) = 0;
	virtual void SetDescriptorSet(RHIRenderingPhase* RenderingPhase, RHIPipeline* Pipeline, RHIDescriptorSet* Set, uint32_t BindIndex, const std::vector<uint32_t>& DynamicOffsets) = 0;
	virtual void DrawMeshTask(RHIRenderingPhase* Phase, uint32_t WorkGroupX, uint32_t WorkGroupY, uint32_t WorkGroupZ) = 0;
	virtual void StartNextSubpass(RHIRenderingPhase* Phase) = 0;
	virtual void EndRenderPass(RHIRenderingPhase* Phase) = 0;
	
	virtual void TranslateImageLayout(
		RHIRenderingPhase* RenderPhase,
		RHIRenderImageAttachment* ToAttachment,
		TexImageLayout Src,
		TexImageLayout Dst,
		PipelineStage WaitSrcStage,
		PipelineStage DstStageWait,
		uint32_t LayerCount,
		uint32_t MipmapCount,
		uint32_t LayerBias = 0,
		uint32_t MipmapBias = 0) = 0;
	/*
	 * Translate layout with access mask manually set
	 */
	virtual void TranslateImageLayoutAccess(
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
		uint32_t LayerBias = 0,
		uint32_t MipmapBias = 0) = 0; 
	
	virtual void SetResizeFunction(const std::function<void(uint32_t NewWidth, uint32_t NewHeight)>& Func);

	virtual void BeginFrameRendering() = 0;
	virtual void EndFrameRendering() = 0;

	/*
	 * Render resource destroyed should no longer used
	 */
	virtual void DestroyTexImageView(RHITexImageView*& Destroyed) = 0;
	virtual void DestroyRenderAttachment(RHIRenderImageAttachment*& Destroyed) = 0;
	virtual void DestroyTexture(RHITexture*& Destroyed) = 0;
	virtual void DestroyBuffer(RHIBuffer*& Destroyed) = 0;
	virtual void DestroyFrameBuffer(RHIFrameBuffer*& Destroyed) = 0;
	virtual void DestroyPipeline(RHIPipeline*& Destroyed) = 0;
	virtual void DestroyRenderPass(RHIRenderPass*& Destroyed) = 0;
	virtual void DestroyDescriptorLayout(RHIDescriptorLayout* Destroyed) = 0;
	virtual void DestroyDescriptorSet(RHIDescriptorSet* Destroyed) = 0;
	virtual void DestroySemaphoreView(RHISemaphoreView* Destroyed) = 0;
	virtual void DestroySemaphore(RHISemaphore* Destroyed) = 0;
	
	virtual void DestroyShader(RHIShaderModule*& Destroyed) = 0;
	
	virtual RHIViewport GetDefaultViewport() const = 0;
	virtual void SetDefaultViewport(const RHIViewport& NewViewport) = 0;
	virtual RHIRect2D GetDefaultScissor() const = 0;
	virtual void SetDefaultScissor(const RHIRect2D& NewScissor) = 0;

	virtual std::string GetRHIName() const = 0;
	virtual std::string GetShaderNameSuffix() const = 0;
};
