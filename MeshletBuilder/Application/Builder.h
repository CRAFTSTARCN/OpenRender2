#pragma once
#include <unordered_map>
#include <unordered_set>

#include "assimp/mesh.h"
#include "OpenRenderRuntime/Core/Basic/Meshlet.h"
class MeshLetBuilder
{

	std::vector<std::vector<uint32_t>> BuildIndexGroup(
		aiMesh* Mesh,
		const std::vector<uint32_t>& Indices);

	/*
	 * Inefficient function
	 */
	uint32_t FindNearestVertexLocation(
		uint32_t FromIndex,
		aiMesh* Mesh, 
		const aiVector3D& Position);

	void BuildGroupWithConnected(
		std::vector<uint32_t>& Last,
		uint32_t StartPosition,
		uint32_t& CurSize,
		uint32_t& CurVertexSize,
		bool& Stop,
		std::unordered_set<uint32_t>& UsedVertex,
		size_t& Remain);

	void BuildMeshletFromIndex(aiMesh* Mesh, const std::vector<uint32_t>& Indices, MeshletPackage& Meshlets);

	
public:

	uint32_t MaxVertexCount = 64;
	uint32_t MaxIndexCount = 126 * 3;

	MeshletPackage BuildMeshlet(aiMesh* Mesh);
	std::unordered_map<uint32_t, std::vector<aiFace*>> VertexUsage;
	std::unordered_set<aiFace*> GlobalUsedFaces;
};
