#include "OpenRenderRuntime/Modules/RenderData/Resolvers/MeshDataResolvers.h"

#include "OpenRenderRuntime/Core/RenderResource/RenderResource.h"
#include "OpenRenderRuntime/Modules/RenderData/MeshSwapData.h"

MeshDataCreateResolver::MeshDataCreateResolver(const RenderComponentsData& InData) : RenderSwapDataResolver(InData)
{
}

void MeshDataCreateResolver::ResolveData(RenderSwapData* Data)
{
	DYNAMIC_CAST_DATA_CHECK(MeshCreateData, Data, MeshData)
	MeshletPackage& Package = MeshData->MeshletPackage;

	/*
	LOG_INFO_FUNCTION(
		"AABB: ({0}, {1}, {2}), ({3}, {4}, {5})",
		MeshData->MeshOriginalAABB.Center.x, MeshData->MeshOriginalAABB.Center.y, MeshData->MeshOriginalAABB.Center.z,
		MeshData->MeshOriginalAABB.Extend.x, MeshData->MeshOriginalAABB.Extend.y, MeshData->MeshOriginalAABB.Extend.z);*/
	
	if(ResourcePtr->Meshes.find(MeshData->MeshId) != ResourcePtr->Meshes.end())
	{
		LOG_ERROR_FUNCTION("Duplicate mesh id");
		delete MeshData;
		return;
	}

	size_t SizeVert = MeshData->VertexData.size() * sizeof(VertexData);
	RHIBuffer* VertexBuffer = RHIPtr->CreateStorageBuffer(SizeVert, BufferMemoryUsage_GPU_Only);
	if(!VertexBuffer)
	{
		LOG_ERROR_FUNCTION("Fail to create vertex buffer");
		delete MeshData;
		return;
	}
	RHIPtr->SetBufferData(VertexBuffer, MeshData->VertexData.data(), SizeVert, 0);
	
	size_t SizeMeshlet = Package.MeshletInfo.size() * sizeof(MeshletDescription);
	RHIBuffer* MeshletBuffer = RHIPtr->CreateStorageBuffer(SizeMeshlet, BufferMemoryUsage_GPU_Only);
	if(!MeshletBuffer)
	{
		LOG_ERROR_FUNCTION("Fail to create meshlet buffer");
		RHIPtr->DestroyBuffer(VertexBuffer);
		delete MeshData;
		return;
	}
	RHIPtr->SetBufferData(MeshletBuffer, Package.MeshletInfo.data(), SizeMeshlet, 0);
	
	size_t SizeVI = Package.VertexIndices.size() * sizeof(uint32_t);
	RHIBuffer* VertexIndexBuffer = RHIPtr->CreateStorageBuffer(SizeVI, BufferMemoryUsage_GPU_Only);
	if(!VertexIndexBuffer)
	{
		LOG_ERROR_FUNCTION("Fail to create prim index buffer");
		RHIPtr->DestroyBuffer(VertexBuffer);
		RHIPtr->DestroyBuffer(MeshletBuffer);
		delete MeshData;
		return;
	}
	RHIPtr->SetBufferData(VertexIndexBuffer, Package.VertexIndices.data(), SizeVI, 0);
	
	size_t SizePI = Package.PrimitiveIndices.size() * sizeof(uint8_t);
	RHIBuffer* PrimIndexBuffer = RHIPtr->CreateStorageBuffer(SizePI, BufferMemoryUsage_GPU_Only);
	if(!PrimIndexBuffer)
	{
		LOG_ERROR_FUNCTION("Fail to create prim index buffer");
		RHIPtr->DestroyBuffer(VertexBuffer);
		RHIPtr->DestroyBuffer(MeshletBuffer);
		RHIPtr->DestroyBuffer(PrimIndexBuffer);
		delete MeshData;
		return;
	}
	RHIPtr->SetBufferData(PrimIndexBuffer, Package.PrimitiveIndices.data(), SizePI, 0);

	RHIDescriptorSet* MeshSet = RHIPtr->CreateDescriptorSet(ResourcePtr->MeshLayout);
	RHIPtr->WriteDescriptorSetMulti(MeshSet, {}, {}, {
		{VertexBuffer, SizeVert, 0, DescriptorType_Storage_Buffer, 0},
		{MeshletBuffer, SizeMeshlet, 0, DescriptorType_Storage_Buffer, 1},
		{VertexIndexBuffer, SizeVI, 0, DescriptorType_Storage_Buffer, 2},
		{PrimIndexBuffer, SizePI, 0, DescriptorType_Storage_Buffer, 3}
	});
	
	RenderMesh* Mesh = new RenderMesh {
		MeshData->MeshId,
		MeshData->MeshOriginalAABB,
		(uint32_t)Package.MeshletInfo.size(),
		VertexBuffer,
		MeshletBuffer,
		VertexIndexBuffer,
		PrimIndexBuffer,
	MeshSet};
	
	ResourcePtr->Meshes.emplace(MeshData->MeshId, Mesh);
	
}

MeshDataDestroyResolver::MeshDataDestroyResolver(const RenderComponentsData& InData) : RenderSwapDataResolver(InData)
{
}

void MeshDataDestroyResolver::ResolveData(RenderSwapData* Data)
{
	DYNAMIC_CAST_DATA_CHECK(MeshDestroyData, Data, DestroyData)

	if(auto Iter = ResourcePtr->Meshes.find(DestroyData->MeshId); Iter == ResourcePtr->Meshes.end())
	{
		LOG_ERROR_FUNCTION("Destroyed mesh not exist");
	}
	else
	{
		ResourcePtr->DestroyMeshResource(Iter->second);
	}

	delete Data;
}
