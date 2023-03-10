#include "OpenRenderRuntime/Core/RenderResource/RenderResource.h"

#include "OpenRenderRuntime/Core/Basic/RenderMath.h"
#include "OpenRenderRuntime/Core/RHI/RHI.h"
#include "OpenRenderRuntime/Modules/RenderConfig/RenderCofig.h"

void RenderResource::Initialize(RHI* InRHI)
{
	RHIPtr = InRHI;

	GlobalBufferSize = RenderConfig::Get().GlobalDCBufferSize;
	GlobalBuffer = RHIPtr->CreateStorageBuffer(
		GlobalBufferSize,
		BufferMemoryUsage_Host_Coherent,
		false);

	if(!GlobalBuffer)
	{
		LOG_ERROR_FUNCTION("Fatal error: fail to create render resrouce global buffer");
		assert(false);
	}

	OnCreateDepthBuffer(RHIPtr->GetSwapchainExtendWidth(), RHIPtr->GetSwapchainExtendHeight());
	OnCreateGBuffer(RHIPtr->GetSwapchainExtendWidth(), RHIPtr->GetSwapchainExtendHeight());
	OnCreateMeshLayout();
	OnCreateGlobalRenderDataBuffer();
}

size_t RenderResource::GetGlobalBufferCurrentOffset()
{
	return CurrentOffset;
}

void RenderResource::AddGlobalBufferOffset(size_t Add)
{
	if(CurrentOffset + Add >=  GlobalBufferSize)
	{
		LOG_ERROR_FUNCTION("Global buffer offset execeed");
		assert(false);
	}

	CurrentOffset += Add;
	CurrentOffset = ORMath::RoundUp(CurrentOffset, GlobalBufferMinAlignment);
}

void RenderResource::SetGlobalDataBuffer(const GlobalRenderDataProxy& Data)
{
	RHIPtr->SetBufferData(GlobalRenderDataBuffer, &Data, sizeof(GlobalRenderDataProxy), 0);
}

void RenderResource::Prepare()
{
	CurrentOffset = 0;
}

void RenderResource::ClearResource()
{
	for(auto [Id, Mesh] : Meshes)
	{
		DestroyMeshResource(Mesh);
	}

	for(auto [Id, MaterialBase] : MaterialBases)
	{
		DestroyMaterialBaseResource(MaterialBase);
	}

	for(auto [Id, MaterialInstance] : Materials)
	{
		DestroyMaterialInstanceResource(MaterialInstance);
	}

	for(auto [Id, Texture] : Textures)
	{
		RHIPtr->DestroyTexture(Texture);
	}

	Meshes.clear();
	MaterialBases.clear();
	Materials.clear();
	Textures.clear();
}

void RenderResource::Terminate()
{
	ClearResource();
	/*
	 * Destroy attachments
	 */
	RHIPtr->DestroyRenderAttachment(GlobalGBuffer.GBufferA);
	RHIPtr->DestroyRenderAttachment(GlobalGBuffer.GBufferB);
	RHIPtr->DestroyRenderAttachment(GlobalGBuffer.GBufferC);
	RHIPtr->DestroyRenderAttachment(GlobalGBuffer.GBufferD);
	RHIPtr->DestroyRenderAttachment(GlobalGBuffer.GBufferE);


	RHIPtr->DestroyRenderAttachment(DepthStencilAttachment);

	/*
	 * Destroy buffer
	 */
	RHIPtr->DestroyBuffer(GlobalBuffer);
	RHIPtr->DestroyBuffer(GlobalRenderDataBuffer);

	RHIPtr->DestroyDescriptorLayout(MeshLayout);
}

void RenderResource::OnCreateGlobalRenderDataBuffer()
{
	GlobalRenderDataBuffer = RHIPtr->CreateUniformBuffer(sizeof(GlobalRenderDataProxy));
}

void RenderResource::OnCreateDepthBuffer(uint32_t Width, uint32_t Height)
{
	DepthStencilAttachment = RHIPtr->CreateRenderAttachment(
		Width,
		Height,
		TexturePixelFormat_D32_FLOAT,
		RenderImageAttachmentType_Depth_Attachment,
		ImageExtraUsageBit_Transfer_Src,1,1,
		true);

	if(!DepthStencilAttachment)
	{
		LOG_ERROR_FUNCTION("Fatal error: fail to create default depth buffer");
	}
}

void RenderResource::OnCreateGBuffer(uint32_t Width, uint32_t Height)
{
	GlobalGBuffer.GBufferA = RHIPtr->CreateRenderAttachment(
		Width,
		Height,
		TexturePixelFormat_RGBA8,
		RenderImageAttachmentType_Color_Attachment,
		0);
	GlobalGBuffer.GBufferB = RHIPtr->CreateRenderAttachment(
		Width,
		Height,
		TexturePixelFormat_RGBA8,
		RenderImageAttachmentType_Color_Attachment,
		0);
	GlobalGBuffer.GBufferC = RHIPtr->CreateRenderAttachment(
		Width,
		Height,
		TexturePixelFormat_RGBA8,
		RenderImageAttachmentType_Color_Attachment,
		0);
	GlobalGBuffer.GBufferD = RHIPtr->CreateRenderAttachment(
		Width,
		Height,
		TexturePixelFormat_RGBA8,
		RenderImageAttachmentType_Color_Attachment,
		0);
	GlobalGBuffer.GBufferE = RHIPtr->CreateRenderAttachment(
		Width,
		Height,
		TexturePixelFormat_RGBA8,
		RenderImageAttachmentType_Color_Attachment,
		0);

	if(!GlobalGBuffer.GBufferA || !GlobalGBuffer.GBufferB || !GlobalGBuffer.GBufferC || !GlobalGBuffer.GBufferD || !GlobalGBuffer.GBufferE)
	{
		LOG_ERROR_FUNCTION("Fail to create gbuffer attachment");
		assert(false);
	}
}

void RenderResource::OnResize(uint32_t Width, uint32_t Height)
{
	RHIPtr->DestroyRenderAttachment(GlobalGBuffer.GBufferA);
	RHIPtr->DestroyRenderAttachment(GlobalGBuffer.GBufferB);
	RHIPtr->DestroyRenderAttachment(GlobalGBuffer.GBufferC);
	RHIPtr->DestroyRenderAttachment(GlobalGBuffer.GBufferD);
	RHIPtr->DestroyRenderAttachment(GlobalGBuffer.GBufferE);


	RHIPtr->DestroyRenderAttachment(DepthStencilAttachment);
	
	OnCreateDepthBuffer(Width, Height);
	OnCreateGBuffer(Width, Height);
}

void RenderResource::OnCreateMeshLayout()
{
	std::vector<DescriptorBindingInfo> Bindings(4);
	DescriptorBindingInfo& VertexBufferBinding = Bindings[0];
	VertexBufferBinding.Type  = DescriptorType_Storage_Buffer;
	VertexBufferBinding.UsageStage = ShaderStageTypeBit_Mesh;

	DescriptorBindingInfo& MeshletBufferBinding = Bindings[1];
	MeshletBufferBinding.Type  = DescriptorType_Storage_Buffer;
	MeshletBufferBinding.UsageStage = ShaderStageTypeBit_Task | ShaderStageTypeBit_Mesh;

	DescriptorBindingInfo& VertexIndexBinding = Bindings[2];
	VertexIndexBinding.Type  = DescriptorType_Storage_Buffer;
	VertexIndexBinding.UsageStage = ShaderStageTypeBit_Mesh;

	DescriptorBindingInfo& PrimIndexBinding = Bindings[3];
	PrimIndexBinding.Type  = DescriptorType_Storage_Buffer;
	PrimIndexBinding.UsageStage = ShaderStageTypeBit_Mesh;

	MeshLayout = RHIPtr->CreateDescriptorLayout(Bindings);
	if(!MeshLayout)
	{
		LOG_ERROR_FUNCTION("Fail to create mesh descriptor layout, fatal");
		assert(false);
	}
}

void RenderResource::DestroyMeshResource(RenderMesh*& Mesh)
{
	RHIPtr->DestroyDescriptorSet(Mesh->Descriptor);
	RHIPtr->DestroyBuffer(Mesh->PrimIndexBuffer);
	RHIPtr->DestroyBuffer(Mesh->VertexIndexBuffer);
	RHIPtr->DestroyBuffer(Mesh->MeshletBuffer);
	RHIPtr->DestroyBuffer(Mesh->VertexBuffer);

	delete Mesh;
}

void RenderResource::DestroyMaterialBaseResource(RenderMaterialBase* MaterialBase)
{
	RHIPtr->DestroyDescriptorLayout(MaterialBase->MaterialParamLayout);
	for(auto Pipeline : MaterialBase->Pipelines)
	{
		if(Pipeline)
		{
			RHIPtr->DestroyPipeline(Pipeline);
		}
	}
	delete MaterialBase;
}

void RenderResource::DestroyMaterialInstanceResource(RenderMaterialInstance* MaterialInstance)
{
	RHIPtr->DestroyDescriptorSet(MaterialInstance->MaterialDescriptorSet);
	RHIPtr->DestroyBuffer(MaterialInstance->MaterialParamUniform);

	delete MaterialInstance;
}
