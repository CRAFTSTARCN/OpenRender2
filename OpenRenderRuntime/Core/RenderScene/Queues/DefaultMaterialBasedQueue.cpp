#include "DefaultMaterialBasedQueue.h"

#include "OpenRenderRuntime/Core/RenderScene/RenderScene.h"

void DefaultMaterialBasedQueue::Form()
{
	if(!Scene)
	{
		return;
	}
	size_t ThisTime = 0;
    
	for(auto & Instance : Scene->Instances)
	{
		if(Instance.IsValid &&
		   (Instance.Visible & 1 << SceneBit) &&
		   (Instance.MaterialPtr->Base->BlendMode == PipelineBlendMode_Opaque || Instance.MaterialPtr->Base->BlendMode == PipelineBlendMode_Masked))
		{
			++ThisTime;
			InsertInstance(Instance, Instance.InstanceID);
		}
	}
    
	static size_t LastTime = 0;

	if(ThisTime != LastTime)
	{
		LOG_INFO_FUNCTION("Current rendering instance count :{0}", ThisTime);
		LastTime = ThisTime;
	}
}

void DefaultMaterialBasedQueue::ResetQueue()
{
	InternalQueue.clear();
}

void DefaultMaterialBasedQueue::InsertInstance(const RenderableInstance& Instance, size_t InstanceIndex)
{
	RenderMesh* Mesh = Instance.MeshPtr;
	RenderMaterialInstance* MaterialInstance = Instance.MaterialPtr;
	RenderMaterialBase* MaterialBase = MaterialInstance->Base;

	MaterialBaseSubQueue& MBQueue = InternalQueue[MaterialBase];
	MaterialSubQueue& MIQueue = MBQueue.MaterialTable[MaterialInstance];
	MeshSubQueue& MeshQueue = MIQueue.MeshTable[Mesh];
	MeshQueue.InstanceIndices.push_back(InstanceIndex);
}


