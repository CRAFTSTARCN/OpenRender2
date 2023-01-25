#pragma once

#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include "MaterialAssets.h"
#include "glm/glm.hpp"

#include "OpenRenderRuntime/Modules/AssetSystem/AssetObject.h"

struct MeshPackageScene
{
	glm::mat4 Transform {};
	std::vector<size_t> MeshIndex;
	std::vector<MeshPackageScene*> Children;
	MeshPackageScene* Parent = nullptr;
};


class MeshPackageAsset : public AssetObject
{

	std::vector<size_t> MeshIds;
	std::vector<MaterialInstanceAsset*> MeshDefaultMaterialPtrs;
	MeshPackageScene* Root = nullptr;

public:

	MeshPackageAsset(std::vector<size_t>&& InMeshes, MeshPackageScene* InSceneRoot = nullptr);

	void SetDefaultMaterials(std::vector<MaterialInstanceAsset*>&& InDefaultMaterials);

	~MeshPackageAsset() override;

	size_t GetMeshCount() const;
	
	size_t GetMeshId(size_t Index) const;

	MaterialInstanceAsset* GetDefaultMaterial(size_t Index) const;
	
	const MeshPackageScene* GetSceneRoot() const;

	static void DeletePackageScene(MeshPackageScene* Scene);

	DEFINE_ASSET_TYPE("StaticMeshPack")
};