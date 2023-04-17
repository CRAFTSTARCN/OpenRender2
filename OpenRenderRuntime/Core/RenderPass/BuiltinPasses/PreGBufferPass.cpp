#include "OpenRenderRuntime/Core/RenderPass/BuiltinPasses/PreGBufferPass.h"

#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/gtc/type_ptr.hpp>

#include "OpenRenderRuntime/Core/RenderPass/BuiltinPasses/MeshDrawCallProxy.h"
#include "OpenRenderRuntime/Core/Basic/RenderMath.h"
#include "OpenRenderRuntime/Core/RenderResource/RenderResource.h"
#include "OpenRenderRuntime/Core/RenderScene/RenderScene.h"
#include "OpenRenderRuntime/Core/RHI/RHI.h"
#include "OpenRenderRuntime/Util/Logger.h"

void PreGBufferPass::CreateRenderPass()
{
	AttachmentDescription ColorAttachment {};
	ColorAttachment.Format = RHIFormat_RGBA8;
	ColorAttachment.Onload = AttachmentLoadOperation_Clear;
	ColorAttachment.OnStore = AttachmentStoreOperation_Store;
	ColorAttachment.InitialLayout = TextureStatus_Undefined;
	ColorAttachment.FinalLayout = TextureStatus_Color_Attachment;

	AttachmentDescription DepthStencilAttachment {};
	DepthStencilAttachment.Format = RHIFormat_D32_FLOAT;
	DepthStencilAttachment.Onload = AttachmentLoadOperation_Clear;
	DepthStencilAttachment.OnStore = AttachmentStoreOperation_Store;
	DepthStencilAttachment.InitialLayout = TextureStatus_Undefined;
	DepthStencilAttachment.FinalLayout = TextureStatus_Depth_Stencil_Attachment;

	SubpassInfo PreGBufferSubpass {};
	for(int i=0; i<5; ++i)
	{
		SubpassAttachmentRef Ref {};
		Ref.Index = i;
		Ref.RequireLayout = TextureStatus_Color_Attachment;
		PreGBufferSubpass.ColorAttachment.push_back(Ref);
	}

	PreGBufferSubpass.DepthStencilAttachment.Index = 5;
	PreGBufferSubpass.DepthStencilAttachment.RequireLayout = TextureStatus_Depth_Stencil_Attachment;
	PreGBufferSubpass.RequireDepthStencil = true;
	
	Pass = RHIPtr->CreateRenderPass(
		{
			ColorAttachment,
			ColorAttachment,
			ColorAttachment,
			ColorAttachment,
			ColorAttachment,
			DepthStencilAttachment},
			{PreGBufferSubpass});

	if(!Pass)
	{
		LOG_ERROR_FUNCTION("Pre gbuffer pass create fail, fatal");
		assert(false);
	}
	
}

void PreGBufferPass::CreateFrameBuffer(uint32_t Width, uint32_t Height)
{
	GBuffer& GlobalGBufferRef = ResourcePtr->GlobalGBuffer;
	FrameBuffer = RHIPtr->CreateFrameBuffer(
		Pass,
		{
			GlobalGBufferRef.GBufferA->DefaultTextureView,
			GlobalGBufferRef.GBufferB->DefaultTextureView,
			GlobalGBufferRef.GBufferC->DefaultTextureView,
			GlobalGBufferRef.GBufferD->DefaultTextureView,
			GlobalGBufferRef.GBufferE->DefaultTextureView,
			ResourcePtr->DepthStencilAttachment->DefaultTextureView},
		Width,
		Height,
		1);

	if(!FrameBuffer)
	{
		LOG_ERROR_FUNCTION("Fail to create pre gbuffer pass frame buffer, fatal");
		assert(false);
	}
}

void PreGBufferPass::CreateGlobalDataSetAndLayout()
{
	
	GlobalDataLayout = RHIPtr->CreateDescriptorLayout({
		{
			DescriptorType_Uniform_Buffer,
			ShaderStageTypeBit_Task | ShaderStageTypeBit_Mesh | ShaderStageTypeBit_Fragment
		}});

	if(!GlobalDataLayout)
	{
		LOG_ERROR_FUNCTION("Fail to create pre gbuffer pass global set layout, fatal");
		assert(false);
	}

	GlobalDataSet = RHIPtr->CreateDescriptorSet(GlobalDataLayout);

	if(!GlobalDataSet)
	{
		LOG_ERROR_FUNCTION("Fail to create pre gbuffer pass descriptor set, fatal");
	}

	RHIPtr->WriteDescriptorSetMulti(GlobalDataSet, {}, {}, {
		{
			ResourcePtr->GlobalRenderDataBuffer,
			sizeof(GlobalRenderDataProxy),
			0,
			DescriptorType_Uniform_Buffer,
			0
		}});
}

void PreGBufferPass::CreateDrawCallDataSetAndLayout()
{
	DrawCallLayout = RHIPtr->CreateDescriptorLayout({
		{
			DescriptorType_Storage_Buffer_Dynamic,
			ShaderStageTypeBit_Task | ShaderStageTypeBit_Mesh
		}});
	
	DrawCallSet = RHIPtr->CreateDescriptorSet(DrawCallLayout);

	if(!DrawCallLayout || !DrawCallSet)
	{
		LOG_ERROR_FUNCTION("Pre gbuffer draw call object create fail, fatal");
		assert(false);
	}

	RHIPtr->WriteDescriptorSetMulti(DrawCallSet, {}, {}, {
		{
			ResourcePtr->GlobalBuffer,
			sizeof(MeshDrawCallDataProxty),
			0,
			DescriptorType_Storage_Buffer_Dynamic,
			0
		}});
	
	ResourcePtr->RegisteredLayout.emplace("DrawCallLayout", DrawCallLayout);
	ResourcePtr->RegisteredSet.emplace("DrawCallSet", DrawCallSet);
}

void PreGBufferPass::LoadPassTaskShader()
{
	std::vector<std::byte> ShaderCode = LoadPassShaderCode("PreGBufferTask.Task");
	if(ShaderCode.size() == 0)
	{
		LOG_ERROR_FUNCTION("Fail to load task shader for pre gbuffer pass");
		assert(false);
	}
	TaskShader = RHIPtr->CreateTaskShader(ShaderCode.data(), ShaderCode.size());
	if(!TaskShader)
	{
		LOG_ERROR_FUNCTION("Fail to load task shader for pre gbuffer pass, fatal");
		assert(false);
	}
}

void PreGBufferPass::CreateSemaphores()
{
	/*
	 * GPU
	 */
	SignaledSemaphore = RHIPtr->CreateSemaphore();
	BBPtr->RegisteredGPUSemaphores.emplace("PreGBufferSemaphore", SignaledSemaphore);

	/*
	 * CPU
	 */
	SubmitSemaphore = new Semaphore;
	BBPtr->RegisteredCPUSemaphores.emplace("PreGBufferSubmit", SubmitSemaphore);
}

PreGBufferPass::PreGBufferPass()
{
	Id = RenderPass::PassIdAllocator.GetNewId();
	MaterialPassId = MeshMaterialPass::MatIdAllocator.GetNewId();
}

PreGBufferPass::~PreGBufferPass()
{
}

void PreGBufferPass::OnResize(uint32_t Width, uint32_t Height)
{
	RHIPtr->DestroyFrameBuffer(FrameBuffer);
	CreateFrameBuffer(Width, Height);	
}

void PreGBufferPass::Initialize()
{

	MeshMaterialPass::Initialize();

	CreateRenderPass();
	CreateGlobalDataSetAndLayout();
	CreateFrameBuffer(RHIPtr->GetSwapchainExtendWidth(), RHIPtr->GetSwapchainExtendHeight());
	LoadPassTaskShader();
	CreateDrawCallDataSetAndLayout();
	CreateSemaphores();
}

void PreGBufferPass::DrawPass()
{
	RHICommandList* CommandList = RHIPtr->GetCommandList(RenderingTaskQueue_Graphics);
	std::vector<ClearColorInfo> ClearColors(6);

	for(size_t i = 0; i < 5; ++i)
	{
		ClearColors[i] = {{0.0f,0.0f,0.0f,0.0f}};
		ClearColors[i].IsDepthStencil = false;
	}

	ClearColors[5].Depth = 1.0f;
	ClearColors[5].Stencil = 0;
	ClearColors[5].IsDepthStencil = true;
	
	RHIPtr->StartRenderPass(CommandList, Pass, FrameBuffer, ClearColors, {
		RHIPtr->GetSwapchainExtendWidth(),
		RHIPtr->GetSwapchainExtendHeight(),
		0,
		0});

	DefaultMaterialBasedQueue& Queue = ScenePtr->PreRenderedOpaquedQueue;

	MeshDrawCallDataProxty DCData {};

	/*
	 * Submit a cluster of instance to command buffer
	 */
	auto SubmitInstances = [&DCData, CommandList, this](uint32_t Cnt, RHIPipeline* Pipeline, RenderMesh* Mesh)
	{
		DCData._darw_instance_count = Cnt;
		DCData._enable_culling = 1;
		size_t CurOffset = 0;
		size_t Size = offsetof(MeshDrawCallDataProxty, _instances) + Cnt * sizeof(InstanceData);
		{
			std::lock_guard<std::mutex> Guard(ResourcePtr->GlobalBufferMutex);
			CurOffset = ResourcePtr->GetGlobalBufferCurrentOffset();
			RHIPtr->SetBufferData(ResourcePtr->GlobalBuffer,
				&DCData,
				Size,
				CurOffset);
			ResourcePtr->AddGlobalBufferOffset(Size);
		}
		RHIPtr->SetDescriptorSet(CommandList, Pipeline, DrawCallSet, 3, {uint32_t(CurOffset)});
		uint32_t TaskCount = ORMath::RoundUp(Mesh->MeshletCount * Cnt, WORKGROUP_SIZE);
		RHIPtr->DrawMeshTask(CommandList, TaskCount >> 5, 1, 1);
	};

	//Per material base (pipeline binding, viewport scissor, and some global data
	for(auto& [MBQMB, MBQ] : Queue.InternalQueue)
	{
		if(MBQMB->Pipelines.size() <= MaterialPassId || !MBQMB->Pipelines[MaterialPassId])
		{
			LOG_ERROR_FUNCTION("No pipeline for this render pass");
			continue;
		}

		RHIPipeline* MaterialPipeline = MBQMB->Pipelines[MaterialPassId];

		RHIPtr->UseGraphicsPipeline(CommandList, MaterialPipeline);
		RHIPtr->SetRenderScissor(CommandList, RHIPtr->GetDefaultScissor(), 0);
		RHIPtr->SetRenderViewport(CommandList, RHIPtr->GetDefaultViewport(), 0);
		RHIPtr->SetDescriptorSet(CommandList, MaterialPipeline, GlobalDataSet, 0, {});

		//Per material instance, binding the descriptor set (param) of each material instance
		for(auto& [MQMI, MQ] : (*MBQ).MaterialTable)
		{
			RHIPtr->SetDescriptorSet(CommandList, MaterialPipeline, MQMI->MaterialDescriptorSet, 2, {});

			//Per mesh, binding mesh's vertex, meshlet, index data
			for(auto& [Mesh, MeshQ] : (*MQ).MeshTable)
			{
				RHIPtr->SetDescriptorSet(CommandList, MaterialPipeline, Mesh->Descriptor, 1, {});

				uint32_t Cnt = 0;
				DCData._meshlet_count = Mesh->MeshletCount;
				for(auto& Instance : (*MeshQ).InstanceIndices)
				{
					RenderableInstance& RI = ScenePtr->Instances[Instance];
					memcpy(DCData._instances[Cnt].mat_model,
						glm::value_ptr(RI.CachedModelMatrix),
						16* sizeof(float));
					glm::mat4 InvTMat = glm::transpose(glm::inverse(RI.CachedModelMatrix));
					memcpy(DCData._instances[Cnt].mat_model_normal,
						glm::value_ptr(InvTMat),
						16* sizeof(float));
					++Cnt;

					if(Cnt == MAX_INSTANCE_DRAW)
					{
						SubmitInstances(Cnt, MaterialPipeline, Mesh);
						Cnt = 0;
					}
				}

				if(Cnt != 0)
				{
					SubmitInstances(Cnt, MaterialPipeline, Mesh);
				}
			}
		}
		
	}
	
	RHIPtr->EndRenderPass(CommandList);

	static size_t FrameNum = 0;
	RHIPtr->SubmitCommandList(CommandList, {}, {/*SignaledSemaphore*/} );
	++FrameNum;
	SubmitSemaphore->Signal();
}

void PreGBufferPass::Terminate()
{
	MeshMaterialPass::Terminate();
	
	RHIPtr->DestroyShader(TaskShader);
	RHIPtr->DestroyFrameBuffer(FrameBuffer);
	RHIPtr->DestroyRenderPass(Pass);

	ResourcePtr->RegisteredLayout.erase("DrawCallSet");
	ResourcePtr->RegisteredLayout.erase("DrawCallLayout");
	RHIPtr->DestroyDescriptorSet(DrawCallSet);
	RHIPtr->DestroyDescriptorLayout(DrawCallLayout);
	RHIPtr->DestroyDescriptorSet(GlobalDataSet);
	RHIPtr->DestroyDescriptorLayout(GlobalDataLayout);

	BBPtr->RegisteredGPUSemaphores.erase("PreGBufferSemaphore");
	RHIPtr->DestroySemaphore(SignaledSemaphore);

	BBPtr->RegisteredCPUSemaphores.erase("PreGBufferSubmit");
}

void PreGBufferPass::OnCreateMaterialBase(MaterialBaseCreateData* Data, RenderMaterialBase* NewMaterialBase)
{
	if(Data->BlendMode == PipelineBlendMode_Translucent || Data->BlendMode == PipelineBlendMode_Additive)
	{
		//No gbuffer pass for translucent or additive material
		return;
	}
	GraphicsPipelineFixedFunctionInfo PipelineInfo {};
	PipelineInfo.ColorBlendingInfos.resize(5); //GBuffer
	
	if(Data->DoubleSided == false)
	{
		PipelineInfo.RasterizationCullFace = CullFace_Back;
	}

	auto Iter = Data->ShaderDataTable.find("GBuffer");
	if(Iter == Data->ShaderDataTable.end())
	{
		//No shader data
		LOG_DEBUG_FUNCTION("Pre gbuffer pass: no shader data for material {}", Data->MaterialBaseId);
		return;
	}

	RHIShaderModule* MeshShader = RHIPtr->CreateMeshShader(
		Iter->second.MeshShaderData.data(),
		Iter->second.MeshShaderData.size());

	if(!MeshShader)
	{
		LOG_ERROR_FUNCTION("Pre gbuffer pass : fail to create mesh shader");
		return;
	}

	RHIShaderModule* FragmentShader = RHIPtr->CreateFragmentShader(
		Iter->second.FragmentShaderData.data(),
		Iter->second.FragmentShaderData.size());

	if(!FragmentShader)
	{
		LOG_ERROR_FUNCTION("Pre gbuffer pass : fail to create fragment shader");
		RHIPtr->DestroyShader(MeshShader);
		return;
	}
	
	RHIPipeline* GBufferPipeline = RHIPtr->CreateGraphicsPipeline(
		Pass,
		0,
		{GlobalDataLayout, ResourcePtr->MeshLayout, NewMaterialBase->MaterialParamLayout, DrawCallLayout},
		{TaskShader, MeshShader, FragmentShader},
		PipelineInfo);

	if(!GBufferPipeline)
	{
		LOG_ERROR_FUNCTION("Pre gbuffer pass: fail to create graphics pipeline");
		return;
	}

	if(NewMaterialBase->Pipelines.size() <= MaterialPassId)
	{
		NewMaterialBase->Pipelines.resize(MaterialPassId+1);
	}

	NewMaterialBase->Pipelines[MaterialPassId] = GBufferPipeline;
	RHIPtr->DestroyShader(MeshShader);
	RHIPtr->DestroyShader(FragmentShader);
}

void PreGBufferPass::OnDestroyMaterialBase(RenderMaterialBase* DestroyedMaterialBase)
{
	if(DestroyedMaterialBase->Pipelines.size() <= MaterialPassId || !DestroyedMaterialBase->Pipelines[MaterialPassId])
	{
		return;
	}

	RHIPipeline* DestroyedPipeline = DestroyedMaterialBase->Pipelines[MaterialPassId];
	RHIPtr->DestroyPipeline(DestroyedPipeline);
}

std::string PreGBufferPass::GetRenderPassName() const
{
	return "PreGBuffer";
}
