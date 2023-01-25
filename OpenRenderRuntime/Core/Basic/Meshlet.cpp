#include "OpenRenderRuntime/Core/Basic/Meshlet.h"

MeshletPackage::MeshletPackage()
{
}

MeshletPackage::MeshletPackage(const MeshletPackage& Copy) :
	MeshletInfo(Copy.MeshletInfo), PrimitiveIndices(Copy.PrimitiveIndices), VertexIndices(Copy.VertexIndices)
{
}

MeshletPackage::MeshletPackage(MeshletPackage&& Move) noexcept :
	MeshletInfo(std::move(Move.MeshletInfo)),
	PrimitiveIndices(std::move(Move.PrimitiveIndices)),
	VertexIndices(std::move(Move.VertexIndices))
{
}

MeshletPackage& MeshletPackage::operator=(const MeshletPackage& Copy)
{
	this->MeshletInfo = Copy.MeshletInfo;
	this->PrimitiveIndices = Copy.PrimitiveIndices;
	this->VertexIndices = Copy.VertexIndices;
	return *this;
}

MeshletPackage& MeshletPackage::operator=(MeshletPackage&& Move) noexcept
{
	this->MeshletInfo = std::move(Move.MeshletInfo);
	this->PrimitiveIndices = std::move(Move.PrimitiveIndices);
	this->VertexIndices = std::move(Move.VertexIndices);
	return *this;
}

