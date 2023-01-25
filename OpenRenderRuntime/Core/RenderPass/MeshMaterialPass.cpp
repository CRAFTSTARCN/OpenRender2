#include "OpenRenderRuntime/Core/RenderPass/MeshMaterialPass.h"

void MeshMaterialPass::Initialize()
{
	RenderPass::Initialize();
	MaterialPassId = MatIdAllocator.GetNewId();
}

void MeshMaterialPass::Terminate()
{
	Ready = false;
	MatIdAllocator.DeallocateId(MaterialPassId);
}

size_t MeshMaterialPass::GetMaterialPassId()
{
	return MaterialPassId;
}
