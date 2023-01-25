#include "DefaultMaterialBasedQueue.h"



void DefaultMaterialBasedQueue::ResetQueue()
{
	InternalQueue.clear();
}

void DefaultMaterialBasedQueue::InsertInstance(const RenderableInstance& Instance, size_t InstanceIndex)
{
	RenderMesh* Mesh = Instance.MeshPtr;
	RenderMaterialInstance* MaterialInstance = Instance.MaterialPtr;
	RenderMaterialBase* MaterialBase = MaterialInstance->Base;

	size_t MeshId = Instance.MeshId;
	size_t MaterialInstanceId = Instance.MaterialId;
	size_t MaterialBaseId = MaterialBase->Id;
	
	auto GIter = InternalQueue.find(MaterialBase->Id); 
	if(GIter == InternalQueue.end())
	{
		InternalQueue.emplace(MaterialBaseId, MaterialBaseSubQueue{{}, MaterialBase});
		GIter = InternalQueue.find(MaterialBaseId);
	}

	auto MatIter = GIter->second.MaterialTable.find(MaterialInstanceId);
	if(MatIter == GIter->second.MaterialTable.end())
	{
		GIter->second.MaterialTable.emplace(MaterialInstanceId, MaterialSubQueue{{}, MaterialInstance});
		MatIter = GIter->second.MaterialTable.find(MaterialInstanceId);
	}

	auto MIter = MatIter->second.MeshTable.find(MeshId);
	if(MIter == MatIter->second.MeshTable.end())
	{
		MatIter->second.MeshTable.emplace(MeshId, MeshSubQueue{{}, Mesh});
		MIter = MatIter->second.MeshTable.find(MeshId);
	}

	MIter->second.MeshIndices.push_back(InstanceIndex);
}


