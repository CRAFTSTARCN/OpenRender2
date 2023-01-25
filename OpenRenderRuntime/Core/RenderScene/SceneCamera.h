#pragma once

#include "OpenRenderRuntime/Core/Basic/Frustum.h"

class SceneCamera
{
    
    glm::mat4 ViewMatrix {};
    glm::mat4 ProjectionMatrix {};
    glm::mat4 VPMatrix {};

    Frustum CameraFrustum {};
    
    glm::vec3 Front {};
    glm::vec3 RightDir {};
    glm::vec3 UpDir {};

    glm::vec3 Position = glm::vec3(0.0f, 0.0f, 0.0f);
    glm::vec3 PitchYawRoll = glm::vec3(0.0f, 90.0f, 0.0f);

    float Near = 0.0f, Far = 10000.0f, FOV = 75.0f, Aspect = 1920.0f / 1080.0f;

    
public:

    void SetupCamera(const glm::vec3& NewPos, const glm::vec3& NewRot,
        float NewNear = -1.0f, float NewFar = -1.0f, float NewFOV = -1.0f, float NewAspect = -1.0f);

    void UpdateViewPositionLook(const glm::vec3& NewPos, const glm::vec3& NewRot);
    
    const glm::mat4& GetView();
    const glm::mat4& GetProjection();
    const glm::mat4& GetVPMatrix();
    const glm::vec3& GetPosition();
    

    Frustum& GetFrustum();
};
