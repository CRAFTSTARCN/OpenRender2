#include "OpenRenderRuntime/Modules/AssetSystem/Importers/MeshImporters.h"

#include <filesystem>
#include <fstream>

#include "assimp/Importer.hpp"
#include "assimp/postprocess.h"
#include "OpenRenderRuntime/Core/RenderResource/RenderMesh.h"
#include "OpenRenderRuntime/Modules/AssetSystem/AssetTypes/MeshAssets.h"
#include "OpenRenderRuntime/Modules/RenderData/MeshSwapData.h"
#include "OpenRenderRuntime/Util/Logger.h"
#include "ThirdParty/assimp/code/AssetLib/Assxml/AssxmlFileWriter.h"

MeshletPackage MeshPackageImporter::DeserializeSingleMeshletGroup(std::ifstream& In)
{
	MeshletPackage Pack {};
	uint32_t Size = 0;
	
	In.read(reinterpret_cast<char*>(&Size), sizeof(uint32_t));
	Pack.MeshletInfo.resize(Size);
	In.read(reinterpret_cast<char*>(Pack.MeshletInfo.data()), (std::streamsize)(Size * sizeof(MeshletDescription)) );

	In.read(reinterpret_cast<char*>(&Size), sizeof(uint32_t));
	Pack.PrimitiveIndices.resize(Size);
	In.read(reinterpret_cast<char*>(Pack.PrimitiveIndices.data()), (std::streamsize)(Size * sizeof(uint8_t)));

	In.read(reinterpret_cast<char*>(&Size), sizeof(uint32_t));
	Pack.VertexIndices.resize(Size);
	In.read(reinterpret_cast<char*>(Pack.VertexIndices.data()), (std::streamsize)(Size * sizeof(uint32_t)));

	return Pack;
}

uint32_t MeshPackageImporter::InitMeshletFileStream(std::ifstream& In, const std::string& FullPath)
{
	In.open(FullPath, std::ios::binary | std::ios::in);
	if(!In.is_open())
	{
		LOG_ERROR_FUNCTION("Fail to open meshlet file {0}", FullPath.c_str());
		return 0;
	}

	uint32_t Head = 0;
	In.read(reinterpret_cast<char*>(&Head), sizeof(uint32_t));
	if(Head != MESHLET_MAGIC_NUMBER)
	{
		LOG_ERROR_FUNCTION("File header not match meshlet file format, {0}", FullPath.c_str());
		In.close();
		return 0;
	}

	uint32_t Size = 0;
	In.read(reinterpret_cast<char*>(&Size), sizeof(uint32_t));
	return Size;
}

std::pair<std::vector<VertexData>, AABB> MeshPackageImporter::AnaSingleMesh(aiMesh* Mesh)
{
	std::vector<VertexData> VertData(Mesh->mNumVertices);
	glm::vec3 MinConor = glm::vec3(std::numeric_limits<float>::max());
	glm::vec3 MaxConor = glm::vec3(-std::numeric_limits<float>::max());

	aiVector3D* Vert = Mesh->mVertices;
	aiVector3D* Norm = Mesh->mNormals;
	aiVector3D* Tan = Mesh->mTangents;
	aiVector3D* UV = Mesh->mTextureCoords[0];
	
	for(uint32_t i=0; i<Mesh->mNumVertices; ++i)
	{
		VertData[i].X = Vert[i].x; VertData[i].Y = Vert[i].y; VertData[i].Z = Vert[i].z;
		VertData[i].NX = Norm[i].x; VertData[i].NY = Norm[i].y; VertData[i].NZ = Norm[i].z;

		if(Tan)
		{
			VertData[i].TX = Tan[i].x; VertData[i].TY = Tan[i].y; VertData[i].TZ = Tan[i].z;
		}
		else
		{
			VertData[i].TX = 1.0f; VertData[i].TY = 0.0f; VertData[i].TZ = 0.0f;
		}

		if(UV)
		{
			VertData[i].U = UV[i].x; VertData[i].V = UV[i].y;	
		}

		MinConor.x = std::min(Vert[i].x, MinConor.x);
		MinConor.y = std::min(Vert[i].y, MinConor.y);
		MinConor.z = std::min(Vert[i].z, MinConor.z);

		MaxConor.x = std::max(Vert[i].x, MaxConor.x);
		MaxConor.y = std::max(Vert[i].y, MaxConor.y);
		MaxConor.z = std::max(Vert[i].z, MaxConor.z);
	}

	std::pair<std::vector<VertexData>, AABB> Res;
	Res.first = std::move(VertData);
	Res.second = AABB(MinConor, MaxConor);

	return Res;
}

MeshPackageScene* MeshPackageImporter::AnaScene(aiNode* Node)
{
	MeshPackageScene* SceneNode = new MeshPackageScene;
	aiMatrix4x4& SceneTransform = Node->mTransformation;
	SceneNode->Transform[0] = glm::vec4(SceneTransform.a1, SceneTransform.b1, SceneTransform.c1, SceneTransform.d1);
	SceneNode->Transform[1] = glm::vec4(SceneTransform.a2, SceneTransform.b2, SceneTransform.c2, SceneTransform.d2);
	SceneNode->Transform[3] = glm::vec4(SceneTransform.a3, SceneTransform.b3, SceneTransform.c3, SceneTransform.d3);
	SceneNode->Transform[3] = glm::vec4(SceneTransform.a4, SceneTransform.b4, SceneTransform.c4, SceneTransform.d4);

	for(uint32_t i=0; i<Node->mNumMeshes; ++i)
	{
		SceneNode->MeshIndex.push_back(Node->mMeshes[i]);
	}

	for(uint32_t i=0; i<Node->mNumChildren; ++i)
	{
		SceneNode->Children.push_back(AnaScene(Node->mChildren[i]));
		SceneNode->Children.back()->Parent = SceneNode;
	}

	return SceneNode;
}

void MeshPackageImporter::SendSingleDestroy2Swap(size_t Id)
{
	MeshDestroyData* SwapData = new MeshDestroyData;
	SwapData->MeshId = Id;
	SwapDataCenterPtr->GetLogicSide().push_back(SwapData);
}

MeshPackageImporter::MeshPackageImporter(const AssetImportData& InData) : AssetImporter(InData)
{
}

MeshPackageImporter::~MeshPackageImporter()
{
}

size_t MeshPackageImporter::LoadAsset(Json AssetJson, const std::string& RelPath)
{
	const std::string& MeshResourcePath = AssetJson["MeshAddress"].string_value();
	if(MeshResourcePath.empty())
	{
		LOG_ERROR_FUNCTION("No valid resource path for {0}, fatal", RelPath.c_str());
		return AssetRegistry::BAD_GASSET_ID;
	}

	std::string MeshFullPath = (std::filesystem::path(ConfigPtr->BasePath) / MeshResourcePath).generic_string();
	Assimp::Importer Importer;
	const aiScene* Scene = Importer.ReadFile(
		MeshFullPath,
		aiProcess_Triangulate |
		aiProcess_JoinIdenticalVertices |
		aiProcess_GenNormals |
		aiProcess_CalcTangentSpace);
	
	if(!Scene || !Scene->mRootNode)
	{
		LOG_ERROR_FUNCTION("Fail to load scene from {0}", MeshResourcePath.c_str());
		return AssetRegistry::BAD_GASSET_ID;
	}

	if(Scene->mNumMeshes == 0)
	{
		LOG_ERROR_FUNCTION("Cannot import empty mesh pack, address {0}", MeshResourcePath.c_str());
		return AssetRegistry::BAD_GASSET_ID;
	}

	std::vector<std::vector<VertexData>> Meshes;
	std::vector<AABB> AABBs;
	std::vector<MeshletPackage> MeshletPacks;

	for(uint32_t i=0; i<Scene->mNumMeshes; ++i)
	{
		auto &&[MeshData, AABBData] = AnaSingleMesh(Scene->mMeshes[i]);
		Meshes.push_back(std::move(MeshData));
		AABBs.push_back(std::move(AABBData));
	}


	const std::string& MeshletPath = AssetJson["MeshletAddress"].string_value();
	std::string MeshletFullPath = (std::filesystem::path(ConfigPtr->BasePath) / MeshletPath).generic_string();
	
	std::ifstream In;
	uint32_t MeshletCount = InitMeshletFileStream(In, MeshletFullPath);

	if(MeshletCount != Scene->mNumMeshes)
	{
		LOG_ERROR_FUNCTION("Meshlet number doesn't fix mesh count, file {0}", MeshletPath.c_str());
		In.close();
		return AssetRegistry::BAD_GASSET_ID;
	}

	for(uint32_t i=0; i<MeshletCount; ++i)
	{
		MeshletPacks.push_back(DeserializeSingleMeshletGroup(In));
	}

	MeshPackageScene* Root = AnaScene(Scene->mRootNode);

	std::vector<size_t> MeshIds(Scene->mNumMeshes);
	for(size_t i=0; i<MeshIds.size(); ++i)
	{
		MeshIds[i] = RenderMesh::Registry.GetNewId();
	}

	MeshPackageAsset* NewAsset = new MeshPackageAsset(std::move(MeshIds), Root);
	size_t GAssetId = RegistryPtr->RegisterNew(NewAsset, RelPath);
	if(GAssetId == AssetRegistry::BAD_GASSET_ID)
	{
		delete NewAsset;
		return AssetRegistry::BAD_GASSET_ID;
	}

	for(uint32_t i=0; i<NewAsset->GetMeshCount(); ++i)
	{
		MeshCreateData* SwapData = new MeshCreateData;
		SwapData->MeshId = NewAsset->GetMeshId(i);
		SwapData->MeshletPackage = std::move(MeshletPacks[i]);
		SwapData->VertexData = std::move(Meshes[i]);
		SwapData->MeshOriginalAABB = AABBs[i];
		SwapDataCenterPtr->GetLogicSide().push_back(SwapData);
	}
	
	const std::vector<Json>& DefaultMatJsonArray = AssetJson["DefaultMaterial"].array_items();
	std::vector<MaterialInstanceAsset*> DefaultMaterials;
	for(uint32_t i=0; i<NewAsset->GetMeshCount(); ++i)
	{
		if(i < DefaultMatJsonArray.size())
		{
			const std::string DefaultMatPath = DefaultMatJsonArray[i].string_value();
			if(DefaultMatPath.empty())
			{
				LOG_WARN_FUNCTION("Missing or invalid path for default material {0}, mesh {1}", RelPath.c_str(), i);
				DefaultMaterials.push_back(nullptr);
			}
			else
			{
				AssetObject* DefaultMatObj = RegistryPtr->TryGetObjectByPath(DefaultMatPath);
				if(DefaultMatObj == nullptr)
				{
					size_t DefaultMatId = OnCascadeLoading(DefaultMatPath);
					if(DefaultMatId == AssetRegistry::BAD_GASSET_ID)
					{
						LOG_WARN_FUNCTION("Default material path {0} not exist", DefaultMatPath.c_str());
						DefaultMaterials.push_back(nullptr);
						continue;
					}
					DefaultMatObj = RegistryPtr->GetObjectById(DefaultMatId);
				}

				MaterialInstanceAsset* DM = dynamic_cast<MaterialInstanceAsset*>(DefaultMatObj);
				if(!DM)
				{
					LOG_ERROR_FUNCTION("Invalid type for default material {0}, require material instance, {1} provided", DefaultMatPath.c_str(), DefaultMatObj->GetAssetTypeName().c_str());
					DefaultMaterials.push_back(nullptr);
				}
				else
				{
					DefaultMaterials.push_back(DM);
					DM->IncreaseUsageCount();
				}
			}
		}
		else
		{
			LOG_WARN_FUNCTION("Missing  path for default material {0}, mesh {1}", RelPath.c_str(), i);
			DefaultMaterials.push_back(nullptr);
		}	
	}

	NewAsset->SetDefaultMaterials(std::move(DefaultMaterials));
	

	return GAssetId;
}

void MeshPackageImporter::UnloadAsset(AssetObject* Asset)
{
	DYNAMIC_CAST_ASSET_CHECK(MeshPackageAsset, Asset, MeshPack)
	for(size_t i=0; i<MeshPack->GetMeshCount(); ++i)
	{
		SendSingleDestroy2Swap(MeshPack->GetMeshId(i));
	}

	RegistryPtr->Unregister(MeshPack->GetId());
	for(size_t i=0; i<MeshPack->GetMeshCount(); ++i)
	{
		MaterialInstanceAsset* DM = MeshPack->GetDefaultMaterial(i);
		if(DM)
		{
			DM->DecreaseUsageCount();
		}
	}
	delete MeshPack;
}
