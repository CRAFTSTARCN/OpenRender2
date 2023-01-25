#include "SceneCamera.h"

#include "OpenRenderRuntime/Core/Basic/RenderMath.h"

#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include "glm/gtc/matrix_transform.hpp"
#include "glm/ext.hpp"

void SceneCamera::SetupCamera(const glm::vec3& NewPos, const glm::vec3& NewRot, float NewNear, float NewFar, float NewFOV,
    float NewAspect)
{

    Position = NewPos;
    PitchYawRoll = NewRot;

    auto CheckSet = [](float& Old, float New)
    {
        if(New >= 0.0f)
        {
            Old = New;
        }
    };

    CheckSet(Near, NewNear);
    CheckSet(Far, NewFar);
    CheckSet(Aspect, NewAspect);
    CheckSet(FOV, NewFOV);
    
    Front.y = sin(glm::radians(PitchYawRoll.x));
    Front.x = cos(glm::radians(PitchYawRoll.x)) * cos(glm::radians(PitchYawRoll.y));
    Front.z = cos(glm::radians(PitchYawRoll.x)) * sin(glm::radians(PitchYawRoll.y));
    Front = glm::normalize(Front);
    UpDir = glm::rotate(glm::vec3(0.0f, 1.0f,0.0f), PitchYawRoll.z, Front);

    RightDir = glm::normalize(glm::cross(Front, UpDir));
    UpDir = glm::cross(RightDir, Front);
    ViewMatrix = glm::lookAt(Position, Position + Front, UpDir);
    ProjectionMatrix = glm::perspective(FOV, Aspect, Near, Far);
    VPMatrix = ProjectionMatrix * ViewMatrix;

    float HT = glm::tan(glm::radians(FOV * 0.5f));
    float HVF = Far * HT;
    float HHF = HVF * Aspect;
    
    
    CameraFrustum = {
        ORMath::GetPlane(glm::cross( Far * Front + HVF * UpDir, RightDir), Position),
        ORMath::GetPlane(glm::cross(RightDir, Far * Front - HVF * UpDir), Position),
        ORMath::GetPlane(glm::cross(Far * Front - RightDir * HHF, UpDir), Position),
        ORMath::GetPlane(glm::cross(Far * Front + RightDir * HHF , -UpDir), Position),
        ORMath::GetPlane(Front, Position + Front * Near),
        ORMath::GetPlane(-Front, Position + Front * Far)
    };
}

void SceneCamera::UpdateViewPositionLook(const glm::vec3& NewPos, const glm::vec3& NewRot)
{
    Position = NewPos;
    PitchYawRoll = NewRot;

    Front.y = sin(glm::radians(PitchYawRoll.x));
    Front.x = cos(glm::radians(PitchYawRoll.x)) * cos(glm::radians(PitchYawRoll.y));
    Front.z = cos(glm::radians(PitchYawRoll.x)) * sin(glm::radians(PitchYawRoll.y));
    Front = glm::normalize(Front);
    UpDir = glm::rotate(glm::vec3(0.0f, 1.0f, 0.0f), PitchYawRoll.z, Front);

    RightDir = glm::normalize(glm::cross(Front, UpDir));
    UpDir = glm::cross(RightDir, Front);
    ViewMatrix = glm::lookAt(Position, Position + Front, UpDir);
    ProjectionMatrix = glm::perspective(FOV, Aspect, Near, Far);
    VPMatrix = ProjectionMatrix * ViewMatrix;

    float HT = glm::tan(glm::radians(FOV * 0.5f));
    float HVF = Far * HT;
    float HHF = HVF * Aspect;
    
    
    CameraFrustum = {
        ORMath::GetPlane(glm::cross( Far * Front + HVF * UpDir, RightDir), Position),
        ORMath::GetPlane(glm::cross(RightDir, Far * Front - HVF * UpDir), Position),
        ORMath::GetPlane(glm::cross(Far * Front - RightDir * HHF, UpDir), Position),
        ORMath::GetPlane(glm::cross(Far * Front + RightDir * HHF , -UpDir), Position),
        ORMath::GetPlane(Front, Position + Front * Near),
        ORMath::GetPlane(-Front, Position + Front * Far)
    };
}

const glm::mat4& SceneCamera::GetView()
{
    return ViewMatrix;
}

const glm::mat4& SceneCamera::GetProjection()
{
    return ProjectionMatrix;
}

const glm::mat4& SceneCamera::GetVPMatrix()
{
    return VPMatrix;
}

const glm::vec3& SceneCamera::GetPosition()
{
    return Position;
}

Frustum& SceneCamera::GetFrustum()
{
    return CameraFrustum;
}
