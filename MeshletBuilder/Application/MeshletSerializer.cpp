#include "MeshletSerializer.h"
#include <fstream>

#include "OpenRenderRuntime/Util/Logger.h"

void MeshletSerializer::SerializeSingleMeshletGroup(const MeshletPackage& MeshletGroup, std::ofstream& Out)
{
	uint32_t Size = (uint32_t)MeshletGroup.MeshletInfo.size();
	Out.write(reinterpret_cast<char*>(&Size), sizeof(uint32_t));
	Out.write(reinterpret_cast<const char*>(MeshletGroup.MeshletInfo.data()), (std::streamsize)Size * (std::streamsize)sizeof(MeshletDescription));

	Size = (uint32_t)MeshletGroup.PrimitiveIndices.size();
	Out.write(reinterpret_cast<char*>(&Size), sizeof(uint32_t));
	Out.write(reinterpret_cast<const char*>(MeshletGroup.PrimitiveIndices.data()), (std::streamsize)Size * (std::streamsize)sizeof(uint8_t));
	
	Size = (uint32_t)MeshletGroup.VertexIndices.size();
	Out.write(reinterpret_cast<char*>(&Size), sizeof(uint32_t));
	Out.write(reinterpret_cast<const char*>(MeshletGroup.VertexIndices.data()), (std::streamsize)Size * (std::streamsize)sizeof(uint32_t));
}

void MeshletSerializer::SerializeMeshlet(const std::vector<MeshletPackage>& MeshletGroups, const std::string& Path)
{
	if(MeshletGroups.empty())
	{
		return;
	}
	std::ofstream Out;
	Out.open(Path, std::ios::out | std::ios::binary);

	if(!Out.is_open())
	{
		LOG_ERROR_FUNCTION("[Meshlet Builder] Fail to open serialize file {0}", Path.c_str());
		return;
	}

	uint32_t Magic = MESHLET_MAGIC_NUMBER;
	Out.write(reinterpret_cast<char*>(&Magic), sizeof(uint32_t));
	uint32_t Size = (uint32_t)MeshletGroups.size();
	Out.write(reinterpret_cast<char*>(&Size), sizeof(uint32_t));

	for(const auto& Meshlet : MeshletGroups)
	{
		SerializeSingleMeshletGroup(Meshlet, Out);
	}

	Out.close();
}
