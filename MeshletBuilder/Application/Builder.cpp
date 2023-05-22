#include "MeshletBuilder/Application/Builder.h"

#include <unordered_set>

#include "OpenRenderRuntime/Util/Logger.h"

glm::vec3 MeshLetBuilder::Ai2GLM(const aiVector3D& AiVector)
{
	return glm::vec3(AiVector.x, AiVector.y, AiVector.z);
}

std::vector<std::vector<uint32_t>> MeshLetBuilder::BuildIndexGroup(aiMesh* Mesh, const std::vector<uint32_t>& Indices)
{
	std::vector<std::vector<uint32_t>> IndexClusters;

	size_t Remain = Indices.size();
	
	for(size_t i=0; i<Indices.size();)
	{
		uint32_t Index = Indices[i];
		auto* CurrentUsage = &VertexUsage[Index];
		while(!CurrentUsage->empty())
		{
			if(!GlobalUsedFaces.empty() && GlobalUsedFaces.find(CurrentUsage->back()) != GlobalUsedFaces.end())
			{
				CurrentUsage->pop_back();
			}
			else
			{
				break;	
			}
		}
		if (CurrentUsage->empty())	
		{
			++i;
			continue;
		}
		
		IndexClusters.push_back({});
		std::vector<uint32_t>& Last = IndexClusters.back();

		uint32_t CurSize = 0;
		uint32_t CurVertexSize = 0;
		std::unordered_set<uint32_t> UsedVertex = {};
		
		{
			for(int j=0; j<3; ++j)
			{
				if(UsedVertex.find(CurrentUsage->back()->mIndices[j]) == UsedVertex.end())
				{
					UsedVertex.insert(CurrentUsage->back()->mIndices[j]);
					++CurVertexSize;
				}

				Last.push_back(CurrentUsage->back()->mIndices[j]);
			}

			GlobalUsedFaces.insert(CurrentUsage->back());
			CurrentUsage->pop_back();
			CurSize += 3;

			if(CurrentUsage->empty())
			{
				--Remain;
			}
		}
		bool Stop = CurSize == MaxIndexCount;

		uint32_t StartPosition = 0;
		
		while(!Stop)
		{
			if(CurSize <= MaxIndexCount || CurVertexSize <= MaxVertexCount)
			{
				BuildGroupWithConnected(
					Last,
					StartPosition,
					CurSize,
					CurVertexSize,
					Stop,
					UsedVertex,
					Remain);
			}

			if(Stop)
			{
				break;
			}

			/*
			 * While we used up all the vertices connected, but not fill a meshlet
			 */

			if(Remain == 0 || CurSize + 3 > MaxIndexCount || CurVertexSize + 3  > MaxVertexCount)
			{
				break;
			}

			uint32_t NextIndex = FindNearestVertexLocation(Mesh, Mesh->mVertices[Last.back()]);

			if(NextIndex == UINT32_MAX)
			{
				break;
			}
			CurrentUsage = &VertexUsage[NextIndex];
			{
				StartPosition = (uint32_t)Last.size();
				for(int j=0; j<3; ++j)
				{
					if(UsedVertex.find(CurrentUsage->back()->mIndices[j]) == UsedVertex.end())
					{
						UsedVertex.insert(CurrentUsage->back()->mIndices[j]);
						++CurVertexSize;
					}

					Last.push_back(CurrentUsage->back()->mIndices[j]);
				}

				GlobalUsedFaces.insert(CurrentUsage->back());
				CurrentUsage->pop_back();
				CurSize += 3;

				if(CurrentUsage->empty())
				{
					--Remain;
				}
			}
		}
	}

	return IndexClusters;
}

uint32_t MeshLetBuilder::FindNearestVertexLocation(aiMesh* Mesh, const aiVector3D& Position)
{

	uint32_t Res = UINT32_MAX;
	float Dist = std::numeric_limits<float>::max();
	
	for(auto Iter = VertexUsage.begin(); Iter != VertexUsage.end();  ++Iter)
	{
		while(!Iter->second.empty())
		{
			if(!GlobalUsedFaces.empty() &&GlobalUsedFaces.find(Iter->second.back()) != GlobalUsedFaces.end())
			{
				Iter->second.pop_back();
			}
			else
			{
				break;
			}
		}
		if(Iter->second.empty())
		{
			continue;
		}
		
		aiVector3D NewLocation = Mesh->mVertices[Iter->first];
		float CurDist = (NewLocation - Position).Length();
		if(Res == UINT32_MAX || Dist > CurDist)
		{
			Dist = CurDist;
			Res = Iter->first;
		}
		
	}

	return Res;
}


void MeshLetBuilder::BuildGroupWithConnected(std::vector<uint32_t>& Last,
                                             uint32_t StartPosition,  uint32_t& CurSize, uint32_t& CurVertexSize, bool& Stop,
                                             std::unordered_set<uint32_t>& UsedVertex, size_t& Remain)
{
	for(; StartPosition<CurSize && !Stop; ++StartPosition)
	{
		std::vector<aiFace*>& CurrentUsage = VertexUsage[Last[StartPosition]];
		while(!CurrentUsage.empty())
		{
			if(!GlobalUsedFaces.empty() && GlobalUsedFaces.find(CurrentUsage.back()) != GlobalUsedFaces.end())
			{
				CurrentUsage.pop_back();
				continue;
			}
			uint32_t NewVertex = 0;
			for(int t = 0; t < 3; ++t)
			{
				if(UsedVertex.find(CurrentUsage.back()->mIndices[t]) == UsedVertex.end())
				{
					UsedVertex.insert(CurrentUsage.back()->mIndices[t]);
					++NewVertex;
				}
			}
			if(CurVertexSize + NewVertex <= MaxVertexCount && CurSize + 3 <= MaxIndexCount)
			{
				CurVertexSize += NewVertex;
				CurSize += 3;
				Last.push_back(CurrentUsage.back()->mIndices[0]);
				Last.push_back(CurrentUsage.back()->mIndices[1]);
				Last.push_back(CurrentUsage.back()->mIndices[2]);
				GlobalUsedFaces.insert(CurrentUsage.back());
				CurrentUsage.pop_back();
				if(CurrentUsage.empty())
				{
					--Remain;
				}
			}
			else
			{
				Stop = true;
				break;
			}
		}
	}
}

void MeshLetBuilder::BuildMeshletFromIndex(aiMesh* Mesh, const std::vector<uint32_t>& Indices, MeshletPackage& Meshlets)
{
	Meshlets.MeshletInfo.push_back({
		((uint32_t)Indices.size() / 3) << 16,
		(uint32_t)Meshlets.VertexIndices.size(), 
		(uint32_t)Meshlets.PrimitiveIndices.size(),
			0, 0, 0, 0, 0, 0, 0, 0});

	std::unordered_map<uint32_t, uint8_t> VertexIndexMap;

	/*
	uint32_t IndexZero = Indices[0];
	float MinX(Mesh->mVertices[IndexZero].x), MinY(Mesh->mVertices[IndexZero].y), MinZ(Mesh->mVertices[IndexZero].z);
	float MaxX(Mesh->mVertices[IndexZero].x), MaxY(Mesh->mVertices[IndexZero].y), MaxZ(Mesh->mVertices[IndexZero].z);
	*/
	
	uint8_t PrimOffsetIndex = 0;
	for(uint32_t i=0; i<Indices.size(); ++i)
	{
		uint32_t CurIndex = Indices[i];
		if(auto Iter = VertexIndexMap.find(CurIndex); Iter == VertexIndexMap.end())
		{
			VertexIndexMap.insert({CurIndex, PrimOffsetIndex});
			Meshlets.VertexIndices.push_back(CurIndex);
			Meshlets.PrimitiveIndices.push_back(PrimOffsetIndex);
			++PrimOffsetIndex;
			/*
			MinX = std::min(Mesh->mVertices[CurIndex].x, MinX);
			MinY = std::min(Mesh->mVertices[CurIndex].y, MinY);
			MinZ = std::min(Mesh->mVertices[CurIndex].z, MinZ);

			MaxX = std::max(Mesh->mVertices[CurIndex].x, MaxX);
			MaxY = std::max(Mesh->mVertices[CurIndex].y, MaxY);
			MaxZ = std::max(Mesh->mVertices[CurIndex].z, MaxZ);
			*/
		}
		else
		{
			Meshlets.PrimitiveIndices.push_back(Iter->second);
		}
	}

	Meshlets.MeshletInfo.back().VertexAndPrimCount |= PrimOffsetIndex;
	BuildBounds(Meshlets.MeshletInfo.back(), Indices, Meshlets.VertexIndices, Mesh->mVertices);

	/*
	Meshlets.MeshletInfo.back().CenterX = (MinX + MaxX) / 2.0f;
	Meshlets.MeshletInfo.back().CenterY = (MinY + MaxY) / 2.0f;
	Meshlets.MeshletInfo.back().CenterZ = (MinZ + MaxZ) / 2.0f;

	Meshlets.MeshletInfo.back().ExtendX = (MaxX - MinX) / 2.0f;
	Meshlets.MeshletInfo.back().ExtendY = (MaxY - MinY) / 2.0f;
	Meshlets.MeshletInfo.back().ExtendZ = (MaxZ - MinZ) / 2.0f;
	*/
}

uint32_t MeshLetBuilder::BuildConeData(float X, float Y, float Z, float CutOff)
{
	uint32_t Res = 0;
	Res |= (uint32_t)SNorm2Uint8(X);
	Res |= ((uint32_t)SNorm2Uint8(Y) << 8);
	Res |= ((uint32_t)SNorm2Uint8(Z) << 16);
	Res |= ((uint32_t)SNorm2Uint8(CutOff) << 24);

	return Res;

}

uint8_t MeshLetBuilder::SNorm2Uint8(float Flt)
{
	float Unsigned = (Flt + 1.0f) / 2.0f;
	uint32_t Val = uint32_t((float)(Unsigned * (float)UINT8_MAX) + 0.5f);
	return (uint8_t)std::min(255u, Val);
}


MeshletPackage MeshLetBuilder::BuildMeshlet(aiMesh* Mesh)
{

	if(!Mesh)
	{
		LOG_ERROR_FUNCTION("No mesh input");
		return {};
	}

	std::vector<uint32_t> Indices;
	for(size_t i=0; i<Mesh->mNumFaces; ++i)
	{
		aiFace& Face = Mesh->mFaces[i];
		if(Face.mNumIndices != 3)
		{
			LOG_ERROR_FUNCTION("Meshlet builder only support triangle polygon");
		}

		for(int j=0; j<3; ++j)
		{
			uint32_t Index = Face.mIndices[j];
			if(auto Iter = VertexUsage.find(Index); Iter == VertexUsage.end())
			{
				VertexUsage.insert({Index, {&Face}});
				Indices.push_back(Index);
			}
			else
			{
				Iter->second.push_back(&Face);
			}
		}
	}

	//Precompute indices
	std::vector<std::vector<uint32_t>> IndexGroups = BuildIndexGroup(Mesh, Indices);

	MeshletPackage Meshlets;
	
	for(uint32_t i=0; i<IndexGroups.size(); ++i)
	{
		BuildMeshletFromIndex(Mesh, IndexGroups[i], Meshlets);
	}
	
	return Meshlets;
}


