#include "PostCombinedPass.h"

#include "OpenRenderRuntime/Core/RenderResource/RenderResource.h"
#include "OpenRenderRuntime/Core/RHI/RHI.h"
#include "OpenRenderRuntime/Util/Logger.h"

void PostCombinedPass::CreateRenderPass()
{
	AttachmentDescription ColorAttachment {};
	ColorAttachment.Format = RHIFormat_RGBA8;
	ColorAttachment.Onload = AttachmentLoadOperation_Load;
	ColorAttachment.OnStore = AttachmentStoreOperation_Nothing;
	ColorAttachment.InitialLayout = TexImageLayout_Color_Attachment;
	ColorAttachment.FinalLayout = TexImageLayout_Shader_TexImage;

	AttachmentDescription DepthStencilAttachment {};
	DepthStencilAttachment.Format = RHIFormat_D32_FLOAT;
	DepthStencilAttachment.Onload = AttachmentLoadOperation_Load;
	DepthStencilAttachment.OnStore = AttachmentStoreOperation_Nothing;
	DepthStencilAttachment.InitialLayout = TexImageLayout_Depth_Stencil_Attachment;
	DepthStencilAttachment.FinalLayout = TexImageLayout_Shader_TexImage;

	AttachmentDescription SwapChainImage {};
	SwapChainImage.Format = RHIPtr->GetSwapchainTextureFormat();
	SwapChainImage.Onload = AttachmentLoadOperation_Clear;
	SwapChainImage.OnStore = AttachmentStoreOperation_Store;
	SwapChainImage.InitialLayout = TexImageLayout_Undefined;
	SwapChainImage.FinalLayout = TexImageLayout_For_Present;

	SubpassInfo PostGBufferSubpass {};
	for(uint32_t i=0; i<5; ++i)
	{
		PostGBufferSubpass.ColorAttachment.push_back({i, TexImageLayout_Color_Attachment});
	}
	PostGBufferSubpass.DepthStencilAttachment = {5u, TexImageLayout_Depth_Stencil_Attachment};
	PostGBufferSubpass.RequireDepthStencil = true;
	PostGBufferSubpass.Dependency.push_back({
		SUBPASS_DEPEND_PASS_EXTERNAL,
		PipelineStageBit_Output_Color,
		PipelineStageBit_Output_Color,
		AccessMaskBit_Color_Attachment_Write,
		AccessMaskBit_Color_Attachment_Write,
		true
	});

	SubpassInfo DeferredShadingSubpass {};
	for(int i=0; i<6; ++i)
	{
		SubpassAttachmentRef Ref {};
		Ref.Index = i;
		Ref.RequireLayout = TexImageLayout_Shader_TexImage;
		DeferredShadingSubpass.InputAttachment.push_back(Ref);
	}
	DeferredShadingSubpass.ColorAttachment.push_back({6, TexImageLayout_Color_Attachment});
	DeferredShadingSubpass.RequireDepthStencil = false;

	DeferredShadingSubpass.Dependency.push_back({
		0,
		PipelineStageBit_Output_Color,
		PipelineStageBit_Fragment,
		AccessMaskBit_Color_Attachment_Write,
		AccessMaskBit_Input_Attachment_Read | AccessMaskBit_Shader_Read,
		true});
	
	DeferredShadingSubpass.Dependency.push_back({
		SUBPASS_DEPEND_PASS_EXTERNAL,
		PipelineStageBit_Output_Color,
		PipelineStageBit_Output_Color,
		0,
		AccessMaskBit_Color_Attachment_Write,
		true});

	Pass = RHIPtr->CreateRenderPass({
		ColorAttachment,
		ColorAttachment,
		ColorAttachment,
		ColorAttachment,
		ColorAttachment,
		DepthStencilAttachment,
		SwapChainImage}, {PostGBufferSubpass, DeferredShadingSubpass});

	if(!Pass)
	{
		LOG_ERROR_FUNCTION("Post combined pass: fail to create render pass");
		assert(false);
	}
}

void PostCombinedPass::CreateDeferredShadingDescriptors()
{
	DeferredShadingGlobalLayout = RHIPtr->CreateDescriptorLayout({
		{DescriptorType_Uniform_Buffer, ShaderStageTypeBit_Fragment},
		{DescriptorType_Texture_With_Sampler, ShaderStageTypeBit_Fragment},
		{DescriptorType_Texture_With_Sampler, ShaderStageTypeBit_Fragment},
		{DescriptorType_Texture_With_Sampler, ShaderStageTypeBit_Fragment},
		{DescriptorType_Texture_With_Sampler, ShaderStageTypeBit_Fragment}
		/*{DescriptorType_Texture, ShaderStageTypeBit_Fragment}*/}); //no shadow currently
	if(!DeferredShadingGlobalLayout)
	{
		LOG_ERROR_FUNCTION("Post combined pass: fail to create descriptor layout for deferred shading");
		assert(false);
	}

	DeferredShadingGlobal = RHIPtr->CreateDescriptorSet(DeferredShadingGlobalLayout);
	if(!DeferredShadingGlobal)
	{
		LOG_ERROR_FUNCTION("Post combined pass: fail to create descriptor set for deferred shading");
		assert(false);
	}

	DeferredShadingGBufferLayout = RHIPtr->CreateDescriptorLayout({
		{DescriptorType_Input_Attachment, ShaderStageTypeBit_Fragment},
		{DescriptorType_Input_Attachment, ShaderStageTypeBit_Fragment},
		{DescriptorType_Input_Attachment, ShaderStageTypeBit_Fragment},
		{DescriptorType_Input_Attachment, ShaderStageTypeBit_Fragment},
		{DescriptorType_Input_Attachment, ShaderStageTypeBit_Fragment},
		{DescriptorType_Input_Attachment, ShaderStageTypeBit_Fragment}});
	if(!DeferredShadingGBufferLayout)
	{
		LOG_ERROR_FUNCTION("Post combined pass: Fail to create descriptor layout for subpass inputs");
		assert(false);
	}

	DeferredShadingGBuffer = RHIPtr->CreateDescriptorSet(DeferredShadingGBufferLayout);
	if(!DeferredShadingGBuffer)
	{
		LOG_ERROR_FUNCTION("Post combined pass: Fail to create descriptor set for subpass inputs");
		assert(false);
	}

	RHIPtr->WriteDescriptorSetMulti(DeferredShadingGlobal, {}, {}, {{
			ResourcePtr->GlobalRenderDataBuffer,
			sizeof(GlobalRenderDataProxy),
			0,
			DescriptorType_Uniform_Buffer,
			0}});
	UpdateGlobalTexture(ResourcePtr->GlobalIBLResource);
	UpdateGBufferAndDepth();
}

void PostCombinedPass::UpdateGBufferAndDepth()
{
	GBuffer& GBufferRef = ResourcePtr->GlobalGBuffer;
	std::vector<RHITexture*> Inputs = {
		GBufferRef.GBufferA,
		GBufferRef.GBufferB,
		GBufferRef.GBufferC,
		GBufferRef.GBufferD,
		GBufferRef.GBufferE,
		ResourcePtr->DepthStencilAttachment};

	std::vector<ImageWriteInfo> ImageWrites(6);
	for(uint32_t i=0; i<6; ++i)
	{
		ImageWrites[i].ImageView = Inputs[i]->DefaultTextureView;
		ImageWrites[i].ImageLayout = TexImageLayout_Shader_TexImage;
		ImageWrites[i].ImageType = DescriptorType_Input_Attachment;
		ImageWrites[i].Binding = i;
		ImageWrites[i].Index = 0;
	}

	RHIPtr->WriteDescriptorSetMulti(DeferredShadingGBuffer, {}, ImageWrites, {});
}

void PostCombinedPass::UpdateGlobalTexture(const IBLResource& IBL)
{
	RenderTexture* SkyBox = IBL.SkyBox;
	RenderTexture* Radiance = IBL.RadianceMap;
	RenderTexture* Irradiance = IBL.IrradianceMap;
	RenderTexture* BRDFLUT = IBL.BRDFLUT;

	std::vector<TextureWithSamplerWriteInfo> WriteTextures;

	auto PushIfNotNull = [&WriteTextures](RenderTexture* Texture, uint32_t BindingPoint)
	{
		if(Texture)
		{
			WriteTextures.push_back({
				Texture->InternalTexture->DefaultTextureView,
				Texture->Sampler,
				BindingPoint,
				0});
		}	
	};

	PushIfNotNull(BRDFLUT, 1);
	PushIfNotNull(Radiance, 2);
	PushIfNotNull(Irradiance, 3);
	PushIfNotNull(SkyBox, 4);
	
	RHIPtr->WriteDescriptorSetMulti(DeferredShadingGlobal, WriteTextures, {}, {});
	
}

void PostCombinedPass::CreateFrameBuffers(uint32_t Width, uint32_t Height)
{
	FrameBuffers.resize(RHIPtr->GetSwapchainImageCount());
	GBuffer& GlobalGBufferRef = ResourcePtr->GlobalGBuffer;

	for(size_t i=0; i<RHIPtr->GetSwapchainImageCount(); ++i)
	{
		FrameBuffers[i] = RHIPtr->CreateFrameBuffer(
			Pass,
			{
				GlobalGBufferRef.GBufferA->DefaultTextureView,
				GlobalGBufferRef.GBufferB->DefaultTextureView,
				GlobalGBufferRef.GBufferC->DefaultTextureView,
				GlobalGBufferRef.GBufferD->DefaultTextureView,
				GlobalGBufferRef.GBufferE->DefaultTextureView,
				ResourcePtr->DepthStencilAttachment->DefaultTextureView,
				RHIPtr->GetSwapchainTexture((uint32_t)i).DefaultTextureView},
			Width,
			Height,
			1);

		if(!FrameBuffers[i])
		{
			LOG_ERROR_FUNCTION("Post combained pass: fail to create frame bufffer");
			assert(false);
		}
	}
}

void PostCombinedPass::CreatePipeline()
{
	std::vector<std::byte> MeshShaderCode = LoadPassShaderCode("DeferredShadingMesh.Mesh");
	std::vector<std::byte> FragShaderCode = LoadPassShaderCode("DeferredShadingFrag.Frag");

	if(MeshShaderCode.empty())
	{
		LOG_ERROR_FUNCTION("Post combined pass: fail to load deferred lighting mesh shader");
		assert(false);
	}

	if(FragShaderCode.empty())
	{
		LOG_ERROR_FUNCTION("Post combined pass: fail to load deferred lighting frag shader");
		assert(false);
	}

	RHIShaderModule*  DeferredShadingMeshShader = RHIPtr->CreateMeshShader(MeshShaderCode.data(), MeshShaderCode.size());
	if(MeshShaderCode.empty())
	{
		LOG_ERROR_FUNCTION("Post combined pass mesh shader create fail");
		assert(false);
	}

	RHIShaderModule* DeferredShadingFragShader = RHIPtr->CreateFragmentShader(FragShaderCode.data(), FragShaderCode.size());
	if(FragShaderCode.empty())
	{
		LOG_ERROR_FUNCTION("Post combined pass fragment shader create fail");
		assert(false);
	}

	GraphicsPipelineFixedFunctionInfo PipelineInfo {};
	PipelineInfo.EnableDepth = false;
	PipelineInfo.EnableDepthWrite = false;
	PipelineInfo.EnableStencil = false;
	PipelineInfo.EnableBlending = false;
	PipelineInfo.RasterizationCullFace = CullFace_None;
	
	PipelineInfo.ColorBlendingInfos.resize(1);

	DeferredShadingPipeline = RHIPtr->CreateGraphicsPipeline(
		Pass,
		1,
		{DeferredShadingGlobalLayout, DeferredShadingGBufferLayout},
		{DeferredShadingMeshShader, DeferredShadingFragShader},
		PipelineInfo);

	if(!DeferredShadingPipeline)
	{
		LOG_ERROR_FUNCTION("Post combine pass: fail to create pipe");
		assert(false);
	}

	RHIPtr->DestroyShader(DeferredShadingMeshShader);
	RHIPtr->DestroyShader(DeferredShadingFragShader);
}

void PostCombinedPass::CreateSyncView()
{
	WaitView = RHIPtr->CreateSemaphoreView(RHIPtr->GetRenderStartSemaphore(), PipelineStageBit_Output_Color);
}


void PostCombinedPass::Initialize()
{
	MeshMaterialPass::Initialize();

	CreateDeferredShadingDescriptors();
	CreateRenderPass();
	CreateFrameBuffers(RHIPtr->GetSwapchainExtendWidth(), RHIPtr->GetSwapchainExtendHeight());
	CreatePipeline();
	CreateSyncView();
}

void PostCombinedPass::DrawPass()
{
	IBLResource& CurrentGlobalIBL = ResourcePtr->GlobalIBLResource;
	RenderTexture* SkyBox = CurrentGlobalIBL.SkyBox;
	RenderTexture* Radiance = CurrentGlobalIBL.RadianceMap;
	RenderTexture* Irradiance = CurrentGlobalIBL.IrradianceMap;
	RenderTexture* BRDFLUT = CurrentGlobalIBL.BRDFLUT;

	if(!SkyBox || !Radiance || !Irradiance || !BRDFLUT)
	{
		LOG_WARN_FUNCTION("Global IBL resource not found, may cause error");
	}
	
	std::vector<ClearColorInfo> ClearColors(7);
	for(int i=0; i<5; ++i)
	{
		ClearColors[i] = {{0.0f,0.0f,0.0f,1.0f}};
		ClearColors[i].IsDepthStencil = false;
	}
	ClearColors[5].Depth = 1.0f;
	ClearColors[5].Stencil = 1;
	ClearColors[5].IsDepthStencil = true;
	ClearColors[6] = {{1.0f,1.0f,1.0f,1.0f}};
	ClearColors[6].IsDepthStencil = false;

	RHICommandList* Phase = RHIPtr->GetCommandList(RenderingTaskQueue_Graphics);
	RHIPtr->StartRenderPass(
		Phase,
		Pass,
		FrameBuffers[RHIPtr->GetCurrentSwapchainImageIndex()],
		ClearColors,
		{RHIPtr->GetSwapchainExtendWidth(), RHIPtr->GetSwapchainExtendHeight(), 0 ,0 });

	RHIPtr->StartNextSubpass(Phase);
	
	RHIPtr->UseGraphicsPipeline(Phase, DeferredShadingPipeline);
	RHIPtr->SetDescriptorSet(Phase, DeferredShadingPipeline, DeferredShadingGlobal, 0, {});
	RHIPtr->SetDescriptorSet(Phase, DeferredShadingPipeline, DeferredShadingGBuffer, 1, {});
	RHIPtr->SetRenderScissor(Phase, RHIPtr->GetDefaultScissor(), 0);
	RHIPtr->SetRenderViewport(Phase, RHIPtr->GetDefaultViewport(), 0);
	RHIPtr->DrawMeshTask(Phase, 1, 1, 1);
	RHIPtr->EndRenderPass(Phase);
	RHIPtr->SubmitCommandList(Phase, {WaitView}, {RHIPtr->GetRenderEndSemaphore()});

	static size_t FrameNum = 0;
	++FrameNum;
}

void PostCombinedPass::Terminate()
{
	MeshMaterialPass::Terminate();

	RHIPtr->DestroyPipeline(DeferredShadingPipeline);
	for(auto FrameBuffer : FrameBuffers)
	{
		RHIPtr->DestroyFrameBuffer(FrameBuffer);
	}
	RHIPtr->DestroyRenderPass(Pass);
	RHIPtr->DestroyDescriptorSet(DeferredShadingGlobal);
	RHIPtr->DestroyDescriptorLayout(DeferredShadingGlobalLayout);
	RHIPtr->DestroyDescriptorSet(DeferredShadingGBuffer);
	RHIPtr->DestroyDescriptorLayout(DeferredShadingGBufferLayout);
	RHIPtr->DestroySemaphoreView(WaitView);
}

void PostCombinedPass::OnCreateMaterialBase(MaterialBaseCreateData* Data, RenderMaterialBase* NewMaterialBase)
{
	//Currently, nothing
}

void PostCombinedPass::OnDestroyMaterialBase(RenderMaterialBase* DestroyedMaterialBase)
{
	//Currently, nothing
}

void PostCombinedPass::OnResize(uint32_t Width, uint32_t Height)
{
	for(auto FrameBuffer : FrameBuffers)
	{
		RHIPtr->DestroyFrameBuffer(FrameBuffer);
	}

	CreateFrameBuffers(Width, Height);
	UpdateGBufferAndDepth();
}

void PostCombinedPass::OnUpdateIBLResource(const IBLResource& IBL)
{
	UpdateGlobalTexture(IBL);
}


std::string PostCombinedPass::GetRenderPassName() const
{
	return "PostCombinedPass";
}
