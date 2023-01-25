#include <glm/gtx/transform.hpp>

#include "OpenRenderRuntime/Core/Basic/RenderMath.h"
#include "OpenRenderRuntime/Core/RenderResource/RenderResource.h"
#include "OpenRenderRuntime/Core/RenderScene/RenderScene.h"
#include "OpenRenderRuntime/Modules/RenderData/RenderableInstanceData.h"
#include "OpenRenderRuntime/Modules/RenderData/Resolvers/RenderableInstanceDataResolvers.h"
#include "OpenRenderRuntime/Util/Logger.h"

RenderableInstanceAddDataResolver::RenderableInstanceAddDataResolver(const RenderComponentsData& InData) : RenderSwapDataResolver(InData)
{
}

void RenderableInstanceAddDataResolver::ResolveData(RenderSwapData* Data)
{
	DYNAMIC_CAST_DATA_CHECK(RenderableInstanceAdd, Data, AddData)

	if(ScenePtr->HasInstance(AddData->RenderableInstanceId))
	{
		LOG_ERROR_FUNCTION("Duplicate instance");
		delete AddData;
		return;
	}

	RenderMesh* Mesh = nullptr;
	if(auto Iter = ResourcePtr->Meshes.find(AddData->MeshId); Iter == ResourcePtr->Meshes.end())
	{
		//Mesh is a necessary resource for instance
		LOG_ERROR_FUNCTION("Fail to find mesh for instance");
		delete AddData;
		return;
	}
	else
	{
		Mesh = Iter->second;
	}

	RenderMaterialInstance* MaterialInstance = nullptr;
	if(auto Iter = ResourcePtr->Materials.find(AddData->MaterialInstanceId); Iter == ResourcePtr->Materials.end())
	{
		//Material instance is not a necessary resource, use default if miss 
		LOG_DEBUG_FUNCTION("Instance's material instance not exist");
		MaterialInstance = ResourcePtr->DefaultResources.DefaultMaterialInstance;
	}
	else
	{
		MaterialInstance = Iter->second;
	}
	
	RenderableInstance Instance {};
	Instance.InstanceID = AddData->RenderableInstanceId;
	Instance.MeshId = AddData->MeshId;
	Instance.MeshPtr = Mesh;
	Instance.MaterialId = AddData->MaterialInstanceId;
	Instance.MaterialPtr = MaterialInstance;

	Instance.CachedModelMatrix = AddData->ModelMatrix;
	Instance.CachedAABB = ORMath::TransformAABBModel(Mesh->OriginalAABB, AddData->ModelMatrix);
	
	Instance.IsValid = true;
	Instance.Visible = 0;
	Instance.IsBlocker = AddData->IsBlocker;

	ScenePtr->TryAddInstance(Instance);
	delete AddData;
}

