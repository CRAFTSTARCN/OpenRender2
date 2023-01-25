#pragma once
#include <cstdint>
#include <vector>

#define MESHLET_MAGIC_NUMBER 0x20221219

/*
 * From nvidia's official suggestion
 * Please use std430 layout!
 */
struct MeshletDescription
{
	uint32_t VertexAndPrimCount;
	uint32_t VertexBegin; 
	uint32_t PrimIndexBegin;
	float CenterX, CenterY, CenterZ;
	float ExtendX, ExtendY, ExtendZ;
};


class MeshletPackage
{

public:
	
	std::vector<MeshletDescription> MeshletInfo;
	std::vector<uint8_t> PrimitiveIndices;
	std::vector<uint32_t> VertexIndices;

	MeshletPackage();
	MeshletPackage(const MeshletPackage& Copy);
	MeshletPackage(MeshletPackage&& Move) noexcept;

	MeshletPackage& operator=(const MeshletPackage& Copy);
	MeshletPackage& operator=(MeshletPackage&& Move) noexcept;
	
};
