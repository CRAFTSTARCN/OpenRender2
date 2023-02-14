#pragma once

#include "OpenRender2/Engine/Core/TransformScript.h"
#include "OpenRenderRuntime/Modules/AssetSystem/AssetTypes/MeshAssets.h"

struct StaticMeshProtoData
{
	std::string MeshPath;
	std::string MaterialOverridePath;
};

class StaticMeshComponent : public TransformScript

{
	
	DECLARE_OBJECT_CLASS(StaticMeshComponent)

protected:

	StaticMeshProtoData* ProtoData;

	MeshPackageAsset* MeshAsset = nullptr;
	MaterialInstanceAsset* Material = nullptr;
	
	uint32_t Index = 0;
	bool IsBlocker = true;
	
	StaticMeshComponent();

public:

	void Serialize(Json ScriptJson) override;

	void ConstructScript() override;

	void Begin() override;
};

