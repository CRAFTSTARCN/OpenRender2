#pragma once
#include "assimp/mesh.h"
#include "OpenRenderRuntime/Core/Basic/BoundingVolumes.h"
#include "OpenRenderRuntime/Core/Basic/VertexData.h"
#include "OpenRenderRuntime/Modules/AssetSystem/AssetImporter.h"
#include "OpenRenderRuntime/Core/Basic/Meshlet.h"

struct MeshPackageScene;

class MeshPackageImporter : public AssetImporter
{

	MeshletPackage DeserializeSingleMeshletGroup(std::ifstream& In);

	uint32_t InitMeshletFileStream(std::ifstream& In, const std::string& FullPath);
	
	std::pair<std::vector<VertexData>, AABB> AnaSingleMesh(aiMesh* Mesh);

	MeshPackageScene* AnaScene(aiNode* Node);

	void SendSingleDestroy2Swap(size_t Id);
	
public:
	
	MeshPackageImporter(const AssetImportData& InData);

	~MeshPackageImporter() override;

	size_t LoadAsset(Json AssetJson, const std::string& RelPath) override;
	
	void UnloadAsset(AssetObject* Asset) override;
};
