#include "DefaultMaterialBasedQueue.h"


void DefaultMaterialBasedQueue::SetScene(RenderScene* InScene)
{
	Scene = InScene;
}

void DefaultMaterialBasedQueue::Form()
{
}

void DefaultMaterialBasedQueue::ResetQueue()
{
	for (auto& MBQ : InternalQueue)
	{
		for(auto& MIQ : MBQ.second->MaterialTable)
		{
			for(auto MQ : MIQ.second->MeshTable)
			{
				delete MQ.second;
			}
			delete MIQ.second;
		}
		delete MBQ.second;
	}

	InternalQueue.clear();
}

void DefaultMaterialBasedQueue::InsertInstance(const RenderableInstance& Instance, size_t InstanceIndex)
{
	RenderMesh* Mesh = Instance.MeshPtr;
	RenderMaterialInstance* MaterialInstance = Instance.MaterialPtr;
	RenderMaterialBase* MaterialBase = MaterialInstance->Base;

	GlobalMutex.lock();
	MaterialBaseSubQueue* MBQueue = InternalQueue[MaterialBase];
	if(!MBQueue)
	{
		MBQueue = new MaterialBaseSubQueue;
		InternalQueue[MaterialBase] = MBQueue;
	}
	GlobalMutex.unlock();

	MBQueue->MaterialBaseMutex.lock();
	MaterialSubQueue* MIQueue = MBQueue->MaterialTable[MaterialInstance];
	if(!MIQueue)
	{
		MIQueue = new MaterialSubQueue;
		MBQueue->MaterialTable[MaterialInstance] = MIQueue;
	}
	MBQueue->MaterialBaseMutex.unlock();

	MIQueue->MaterialInstanceMutex.lock();
	MeshSubQueue* MeshQueue = MIQueue->MeshTable[Mesh];

	if(!MeshQueue)
	{
		MeshQueue = new MeshSubQueue;
		MIQueue->MeshTable[Mesh] = MeshQueue;
	}
	MIQueue->MaterialInstanceMutex.unlock();

	MeshQueue->MeshMutex.lock();
	MeshQueue->InstanceIndices.push_back(InstanceIndex);
	MeshQueue->MeshMutex.unlock();
}


