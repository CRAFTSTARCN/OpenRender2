#include "OpenRenderRuntime/Util/Logger.h"

#include <filesystem>

#include "Application/Builder.h"
#include "Application/MeshletSerializer.h"
#include "assimp/Importer.hpp"
#include "assimp/postprocess.h"
#include "assimp/scene.h"


int main(int argc, char* argv[])
{
	std::filesystem::path ExeDir = std::filesystem::path(argv[0]).parent_path();
	LOG_INFO_FUNCTION(ExeDir.generic_string());

	//Testing

	std::filesystem::path Path = (ExeDir / "Content/Mesh/Bunny/Bunny.fbx");
	
	Assimp::Importer Importer;
	const aiScene* Scene = Importer.ReadFile(
		Path.generic_string().c_str(),
		aiProcess_Triangulate | aiProcess_JoinIdenticalVertices | aiProcess_GenNormals | aiProcess_CalcTangentSpace);
	if(!Scene || !Scene->mRootNode)
	{
		return 0;
	}

	LOG_INFO("Scene mesh count {}", Scene->mNumMeshes);

	std::vector<MeshletPackage> MeshletGroups;
	for(uint32_t i = 0; i<Scene->mNumMeshes; ++i)
	{
		MeshLetBuilder Builder;
		aiMesh* Mesh = Scene->mMeshes[i];
		MeshletGroups.push_back(Builder.BuildMeshlet(Mesh));
	}

	MeshletSerializer Serializer;
	Serializer.SerializeMeshlet(MeshletGroups, (ExeDir / "Content/Mesh/Bunny/Bunny.mlt").generic_string());
	
	return 0;
}
