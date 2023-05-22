#include "Builder.h"

/*
 * From meshoptimize
 */
glm::vec4 MeshLetBuilder::GetBoundingSphere(const glm::vec3* Points, uint32_t Count)
{
    if(Count == 0)
    {
        return glm::vec4(0.0);
    }

    size_t PointMin[3] = {0, 0, 0};
    size_t PointMax[3] = {0, 0, 0};

    for (size_t i = 0; i < Count; ++i)
    {
        const glm::vec3& P = Points[i];

        for (int C = 0; C < 3; ++C)
        {
            PointMin[C] = (P[C] < Points[PointMin[C]][C]) ? i : PointMin[C];
            PointMax[C] = (P[C] > Points[PointMax[C]][C]) ? i : PointMax[C];
        }
    }

    float MaxAxisD   = 0;
    int MaxAxisIndex = 0;

    for (int axis = 0; axis < 3; ++axis)
    {
        const glm::vec3& PMin = Points[PointMin[axis]];
        const glm::vec3& PMax = Points[PointMax[axis]];

        
        float D2 = glm::length(PMax - PMin);

        if (D2 > MaxAxisD)
        {
            MaxAxisD = D2;
            MaxAxisIndex = axis;
        }
    }

    // use the longest segment as the initial sphere diameter
    const glm::vec3 PMin = Points[PointMin[MaxAxisIndex]];
    const glm::vec3 PMax= Points[PointMax[MaxAxisIndex]];

    glm::vec3 ResC = glm::vec3(
        (PMin[0] + PMax[0]) / 2,
        (PMin[1] + PMax[1]) / 2,
        (PMin[2] + PMax[2]) / 2);

    float ResRadius = sqrtf(MaxAxisD) / 2;


    // iteratively adjust the sphere up until all points fit
    for (size_t i = 0; i < Count; ++i)
    {
        const glm::vec3& P = Points[i];
        
        float D2 = glm::length(P - ResC);

        if (D2 > ResRadius * ResRadius)
        {
            float D = sqrtf(D2);
            assert(D > 0);

            float k = 0.5f + (ResRadius / D) / 2;
            ResC = (ResC * k) + (P * (1 - k));
            ResRadius = (ResRadius + D) / 2;
        }
    }

    return glm::vec4(ResC.x, ResC.y, ResC.z, ResRadius);
}


void MeshLetBuilder::BuildBounds(MeshletDescription& ToDesc, const std::vector<uint32_t>& Indices,
    const std::vector<uint32_t>& UsedVertexIndices, aiVector3D* Vertices)
{
    ToDesc.Cone = 0;
    size_t PrimCount = Indices.size() / 3;

    uint32_t StartIndex = ToDesc.VertexBegin;
    float MinX(std::numeric_limits<float>::max()), MinY(std::numeric_limits<float>::max()), MinZ(std::numeric_limits<float>::max());
    float MaxX(std::numeric_limits<float>::lowest()), MaxY(std::numeric_limits<float>::lowest()), MaxZ(std::numeric_limits<float>::lowest());
    for(size_t i = StartIndex; i < UsedVertexIndices.size(); ++i)
    {
        uint32_t CurIndex = UsedVertexIndices[i];
        MinX = std::min(Vertices[CurIndex].x, MinX);
        MinY = std::min(Vertices[CurIndex].y, MinY);
        MinZ = std::min(Vertices[CurIndex].z, MinZ);

        MaxX = std::max(Vertices[CurIndex].x, MaxX);
        MaxY = std::max(Vertices[CurIndex].y, MaxY);
        MaxZ = std::max(Vertices[CurIndex].z, MaxZ);
    }

    ToDesc.CenterX = (MinX + MaxX) / 2.0f;
    ToDesc.CenterY = (MinY + MaxY) / 2.0f;
    ToDesc.CenterZ = (MinZ + MaxZ) / 2.0f;

    ToDesc.ExtendX = (MaxX - MinX) / 2.0f;
    ToDesc.ExtendY = (MaxY - MinY) / 2.0f;
    ToDesc.ExtendZ = (MaxZ - MinZ) / 2.0f;
    
    std::vector<glm::vec3> Normals(PrimCount);

    for(size_t i = 0; i < PrimCount; ++i)
    {
        const aiVector3D& P0 = Vertices[Indices[i * 3]];
        const aiVector3D& P1 = Vertices[Indices[i * 3 + 1]];
        const aiVector3D& P2 = Vertices[Indices[i * 3 + 2]];

        glm::vec3 P10 = Ai2GLM(P1 - P0);
        glm::vec3 P20 = Ai2GLM(P2 - P0);
        Normals[i] = glm::normalize(glm::cross(P10, P20)); //CCW
    }
    
    glm::vec4 NormalSphere = GetBoundingSphere(Normals.data(), (uint32_t)Normals.size());
    glm::vec3 Axis = glm::normalize(glm::vec3(NormalSphere));

    float MinDp = 1.0f;

    for(size_t i = 0; i < PrimCount; ++i)
    {
        float Dp = glm::dot(Normals[i], Axis);
        if(Dp < MinDp)
        {
            MinDp = Dp;
        }
    }

    if(MinDp < 0.1f)
    {
        
        ToDesc.Cone = BuildConeData(0.0f,0.0f,0.0f,1.0f);
        ToDesc.ApexOffset = 0.0;
        return;
    }

    float MaxT = 0;
    glm::vec3 Center = glm::vec3(ToDesc.CenterX, ToDesc.CenterY, ToDesc.CenterZ);
    for (size_t i = 0; i < PrimCount; ++i)
    {
        
        glm::vec3 T0 = Ai2GLM(Vertices[Indices[i * 3]]);
        
        glm::vec3 C = Center - T0;

        glm::vec3 Normal = Normals[i];
        float Dc = glm::dot(C, Normal);
        float Dn = glm::dot(Axis, Normal);

        assert(Dn > 0.0f);
        float T = Dc / Dn;

        if(T > MaxT)
        {
            MaxT = T;
        }
    }


    ToDesc.ApexOffset = MaxT;
    float CutOff = std::sqrtf(1.0f - MinDp * MinDp);
    ToDesc.Cone = BuildConeData(Axis.x, Axis.y, Axis.z, CutOff);
    return;
}


