#pragma once
#include "OpenRenderRuntime/Core/RenderResource/RenderMaterialInstance.h"
#include "OpenRenderRuntime/Core/RenderResource/RenderMesh.h"

class RenderableInstance
{
    
public:

    inline static AutoIncreaseIdAllocator Registry {0};
    
    size_t InstanceID = 0;
    glm::mat4 CachedModelMatrix = glm::mat4(1);
    
    size_t MeshId = 0;
    RenderMesh* MeshPtr = nullptr;

    size_t MaterialId = 0;
    RenderMaterialInstance* MaterialPtr = nullptr;

    size_t Visible = 0;
    AABB CachedAABB = AABB(glm::vec3(0.0f,0.0f,0.0f), glm::vec3(0.0f,0.0f,0.0f));

    bool IsBlocker = false;
    bool IsValid = true;
    
    //For future usage
    float Time = 0.0f;
};
