#pragma once
#include <string>

#include "OpenRenderRuntime/Core/Basic/Meshlet.h"

class MeshletSerializer
{

	void SerializeSingleMeshletGroup(const MeshletPackage& MeshletGroup, std::ofstream& Out);
	
public:
	
	/*
	 * Meshlet will be serialized into binary form
	 */
	void SerializeMeshlet(const std::vector<MeshletPackage>& MeshletGroups, const std::string& Path);
};
