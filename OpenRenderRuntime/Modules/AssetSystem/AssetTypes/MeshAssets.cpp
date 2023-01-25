#include "OpenRenderRuntime/Modules/AssetSystem/AssetTypes/MeshAssets.h"

#include "OpenRenderRuntime/Util/Logger.h"


MeshPackageAsset::MeshPackageAsset(std::vector<size_t>&& InMeshes, MeshPackageScene* InSceneRoot) :
	MeshIds(std::move(InMeshes)),  Root(InSceneRoot)
{
}

void MeshPackageAsset::SetDefaultMaterials(std::vector<MaterialInstanceAsset*>&& InDefaultMaterials)
{
	if(InDefaultMaterials.size() < MeshIds.size())
	{
		LOG_WARN_FUNCTION("Less material detected, default material might used");
	}
	else if(InDefaultMaterials.size() > MeshIds.size())
	{
		LOG_WARN_FUNCTION("More material detected, some will ignored");
	}

	MeshDefaultMaterialPtrs = std::move(InDefaultMaterials);
	while(MeshDefaultMaterialPtrs.size() > MeshIds.size())
	{
		MeshDefaultMaterialPtrs.pop_back();
	}
}

MeshPackageAsset::~MeshPackageAsset()
{
	MeshPackageAsset::DeletePackageScene(Root);
}

size_t MeshPackageAsset::GetMeshCount() const
{
	return MeshIds.size();
}


size_t MeshPackageAsset::GetMeshId(size_t Index) const
{
	if(Index >= MeshIds.size())
	{
		LOG_ERROR_FUNCTION("Mesh package index out of range");
		return SIZE_MAX;
	}
	return MeshIds[Index];
}

MaterialInstanceAsset* MeshPackageAsset::GetDefaultMaterial(size_t Index) const
{
	return Index >= MeshDefaultMaterialPtrs.size() ? nullptr : MeshDefaultMaterialPtrs[Index];
}

const MeshPackageScene* MeshPackageAsset::GetSceneRoot() const
{
	return Root;
}

void MeshPackageAsset::DeletePackageScene(MeshPackageScene* Scene)
{
	for(auto Child : Scene->Children)
	{
		DeletePackageScene(Child);
	}
	delete Scene;
}
