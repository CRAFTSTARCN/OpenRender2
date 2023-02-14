#include "OpenRender2/Engine/CoreObjects/StaticMeshComponent.h"
#include "OpenRender2/Engine/Core/ObjectClass.h"
#include "OpenRender2/Engine/Core/OpenRenderEngine.h"
#include "OpenRenderRuntime/Modules/RenderData/RenderableInstanceData.h"


StaticMeshComponent::StaticMeshComponent() : ProtoData(new StaticMeshProtoData)
{
	
}

void StaticMeshComponent::Serialize(Json ScriptJson)
{
	TransformScript::Serialize(ScriptJson);
	ProtoData->MeshPath = ScriptJson["StaticMesh"].string_value();
	ProtoData->MaterialOverridePath = ScriptJson["MaterialOverride"].string_value();
	Index = ScriptJson["PackIndex"].int_value();
	IsBlocker = ScriptJson["Blocker"].bool_value();
}

void StaticMeshComponent::ConstructScript()
{
	TransformScript::ConstructScript();

	//Mesh asset
	size_t AssetId = GEngine->GetAssetSystem()->GetOrImport(ProtoData->MeshPath);
	if(AssetId == AssetRegistry::BAD_GASSET_ID)
	{
		LOG_ERROR_FUNCTION("Fail to load mesh asset, component : {0}, path : {1}", Name.c_str(), ProtoData->MeshPath.c_str());
		return;
	}

	MeshAsset = dynamic_cast<MeshPackageAsset*>(GEngine->GetAssetSystem()->GetAssetById(AssetId));
	if(!MeshAsset)
	{
		LOG_ERROR_FUNCTION("Error, asset type, {0}", ProtoData->MeshPath);
		return;
	}

	MeshAsset->IncreaseUsageCount();
	
	if(Index >= MeshAsset->GetMeshCount())
	{
		LOG_ERROR_FUNCTION("Index out of range for mesh component {0}, {1} pack only has {2} mesh will use 0",
							Name.c_str(), ProtoData->MeshPath.c_str(), MeshAsset->GetMeshCount());
		Index = 0;
	}

	//Material asset
	if(ProtoData->MaterialOverridePath.empty())
	{
		Material = MeshAsset->GetDefaultMaterial(Index);
	}

	else
	{
		size_t MatId = GEngine->GetAssetSystem()->GetOrImport(ProtoData->MaterialOverridePath);

		if(MatId == AssetRegistry::BAD_GASSET_ID)
		{
			LOG_ERROR_FUNCTION("Fail to load material {0}", ProtoData->MaterialOverridePath.c_str());
			return;
		}

		Material = dynamic_cast<MaterialInstanceAsset*>(GEngine->GetAssetSystem()->GetAssetById(MatId));
		if(!Material)
		{
			LOG_ERROR_FUNCTION("Wrong type for material, path {0}", ProtoData->MaterialOverridePath);
			return;
		}

		Material->IncreaseUsageCount();
		
	}

	if(Material)
	{
		Material->IncreaseUsageCount();
	}
}

void StaticMeshComponent::Begin()
{
	TransformScript::Begin();

	if(MeshAsset)
	{
		RenderableInstanceAdd* AddStaticMesh = new RenderableInstanceAdd;
		AddStaticMesh->RenderableInstanceId = RenderableInstance::Registry.GetNewId();
		AddStaticMesh->MeshId = MeshAsset->GetMeshId(Index);
		AddStaticMesh->MaterialInstanceId = Material ? Material->GetMaterialInstanceId() : RenderMaterialInstance::BAD_MATERIAL_INSTANCE_ID;
		AddStaticMesh->ModelMatrix = GetTransformMatAbs();
		AddStaticMesh->IsBlocker = IsBlocker;

		GEngine->GetSwapDataCenter()->GetLogicSide().push_back(AddStaticMesh);
	}
	
	
}


DEFINE_OBJECT_CLASS(StaticMeshComponent)
