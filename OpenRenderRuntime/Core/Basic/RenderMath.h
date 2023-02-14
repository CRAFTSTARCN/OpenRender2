#pragma once
#include "OpenRenderRuntime/Core/Basic/BoundingVolumes.h"
#include "OpenRenderRuntime/Core/Basic/Frustum.h"
#include "OpenRenderRuntime/Core/RenderScene/Lights.h"
#include "OpenRenderRuntime/Core/Basic/Transform.h"

class ORMath
{
    
public:

    template <typename T>
    inline static T RoundUp(T Value, T Base)
    {
        T Temp = Value + Base - 1;
        return Temp - (Temp % Base);
    }
    static glm::vec4 GetPlane(const glm::vec3& Normal, const glm::vec3& Point);
    static bool AABBIntersectsFrustum(const AABB& Bounding, const Frustum& F);
    static AABB TransformAABB(const AABB& Original, const glm::mat4& TransformMatrix);
    /*
     * Model matrix transform
     * Faster than use any matrix transform
     */
    static AABB TransformAABBModel(const AABB& Original, const glm::mat4& ModelMatrix);
    static bool AABBIntersectsHalfSpace(const AABB& Bounding, const glm::vec4& CutPlane);
    static glm::mat4 GetDirectionalViewProj(
        DirectionalLight& Light,
        glm::vec3 CamLocation,
        float UpDist,
        float Far,
        float ExtendH,
        float ExtendV);

    static glm::mat4 Transform2Matrix(const Transform& FromTransform);

    static float GetDistToPlane(const glm::vec4& Plane, const glm::vec3& Point);

    static Transform GetIdentityTransform();
};
