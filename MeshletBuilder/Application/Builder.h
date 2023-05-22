#pragma once

#define GLM_FORCE_DEPTH_ZERO_TO_ONE

#include <unordered_map>
#include <unordered_set>
#include <glm/glm.hpp>

#include "assimp/mesh.h"
#include "OpenRenderRuntime/Core/Basic/Meshlet.h"
class MeshLetBuilder
{

	glm::vec3 Ai2GLM(const aiVector3D& AiVector);
	
	std::vector<std::vector<uint32_t>> BuildIndexGroup(
		aiMesh* Mesh,
		const std::vector<uint32_t>& Indices);

	/*
	 * Inefficient function
	 */
	uint32_t FindNearestVertexLocation(
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

	uint32_t BuildConeData(float X, float Y, float Z, float CutOff);

	uint8_t SNorm2Uint8(float Flt);
	
public:

	uint32_t MaxVertexCount = 64;
	uint32_t MaxIndexCount = 126 * 3;

	MeshletPackage BuildMeshlet(aiMesh* Mesh);

	void BuildBounds(MeshletDescription& ToDesc, const std::vector<uint32_t>& Indices, const std::vector<uint32_t>& UsedVertexIndices, aiVector3D* Vertices);

	glm::vec4 GetBoundingSphere(const glm::vec3* Points, uint32_t Count);
	
	std::unordered_map<uint32_t, std::vector<aiFace*>> VertexUsage;
	std::unordered_set<aiFace*> GlobalUsedFaces;
};
