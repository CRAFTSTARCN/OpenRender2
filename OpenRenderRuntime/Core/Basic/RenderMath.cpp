#include "RenderMath.h"

#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/gtc/matrix_transform.hpp>


glm::vec4 ORMath::GetPlane(const glm::vec3& Normal, const glm::vec3& Point)
{
    glm::vec3 Normalized = glm::normalize(Normal);
    return glm::vec4(Normalized, glm::dot(Normalized, Point));
}

bool ORMath::AABBIntersectsFrustum(const AABB& Bounding, const Frustum& F)
{

    /*
    if(!AABBIntersectsHalfSpace(Bounding, F.Top))
    {
        LOG_INFO("Miss top");
        return false;
    }

    if(!AABBIntersectsHalfSpace(Bounding, F.Down))
    {
        LOG_INFO("Miss down");
        return false;
    }
    
    if(!AABBIntersectsHalfSpace(Bounding, F.Left))
    {
        LOG_INFO("Miss left");
        return false;
    }
    
    if(!AABBIntersectsHalfSpace(Bounding, F.Right))
    {
        LOG_INFO("Miss right");
        return false;
    }
    
    if(!AABBIntersectsHalfSpace(Bounding, F.Near))
    {
        LOG_INFO("Miss near");
        return false;
    }
    
    if(!AABBIntersectsHalfSpace(Bounding, F.Far))
    {
        LOG_INFO("Miss far");
        return false;
    }

    return true;*/
    
    return AABBIntersectsHalfSpace(Bounding, F.Top) &&
           AABBIntersectsHalfSpace(Bounding, F.Down) &&
           AABBIntersectsHalfSpace(Bounding, F.Left) &&
           AABBIntersectsHalfSpace(Bounding, F.Right) &&
           AABBIntersectsHalfSpace(Bounding, F.Near) &&
           AABBIntersectsHalfSpace(Bounding, F.Far);
}

AABB ORMath::TransformAABB(const AABB& Original, const glm::mat4& TransformMatrix)
{
    glm::vec3 Min{
        std::numeric_limits<float>::max(),
        std::numeric_limits<float>::max(),
        std::numeric_limits<float>::max()};
    glm::vec3 Max{
        -std::numeric_limits<float>::max(),
        -std::numeric_limits<float>::max(),
        -std::numeric_limits<float>::max()};

    auto CheckMinMax = [](float& MinVal, float& MaxVal, float Current)
    {
        if(Current < MinVal)
        {
            MinVal = Current;
        }

        if(Current > MaxVal)
        {
            MaxVal = Current;
        }
    };
    
    for(int i=0; i<8; ++i)
    {
        glm::vec3 Mul;
        Mul.x = (i & 1) == 1 ? 1 : -1;
        Mul.y = ((i >> 1) & 1) == 1 ? 1 : -1;
        Mul.z = ((i >> 2) & 1) == 1 ? 1 : -1;
        glm::vec4 V = glm::vec4(Original.Center + Mul * Original.Extend, 1.0f);

        V = TransformMatrix * V;
        V.x /= V.w;
        V.y /= V.w;
        V.z /= V.w;

        CheckMinMax(Min.x, Max.x, V.x);
        CheckMinMax(Min.y, Max.y, V.y);
        CheckMinMax(Min.z, Max.z, V.z);
    }

    return AABB(Min, Max);
}

AABB ORMath::TransformAABBModel(const AABB& Original, const glm::mat4& ModelMatrix)
{

    glm::vec4 WorldCenter = ModelMatrix * glm::vec4(Original.Center, 1.0f);
    
    glm::vec3 Right = glm::vec3(ModelMatrix[0]) * Original.Extend.x;
    glm::vec3 Up = glm::vec3(ModelMatrix[1]) * Original.Extend.y;
    glm::vec3 Forward = glm::vec3(ModelMatrix[2]) * Original.Extend.z;

    float WorldI = std::abs(glm::dot(glm::vec3(1.0f, 0.0f, 0.0f), Right)) +
                   std::abs(glm::dot(glm::vec3(1.0f, 0.0f, 0.0f), Forward)) +
                   std::abs(glm::dot(glm::vec3(1.0f, 0.0f, 0.0f), Up));

    float WorldJ = std::abs(glm::dot(glm::vec3(0.0f, 1.0f, 0.0f), Right)) +
                   std::abs(glm::dot(glm::vec3(0.0f, 1.0f, 0.0f), Forward)) +
                   std::abs(glm::dot(glm::vec3(0.0f, 1.0f, 0.0f), Up));

    float WorldK = std::abs(glm::dot(glm::vec3(0.0f, 0.0f, 1.0f), Right)) +
                   std::abs(glm::dot(glm::vec3(0.0f, 0.0f, 1.0f), Forward)) +
                   std::abs(glm::dot(glm::vec3(0.0f, 0.0f, 1.0f), Up));

    AABB Res;
    Res.Center = glm::vec3(WorldCenter);
    Res.Extend = glm::vec3(WorldI, WorldJ, WorldK);

    return Res;
}

bool ORMath::AABBIntersectsHalfSpace(const AABB& Bounding, const glm::vec4& CutPlane)
{
    float R = Bounding.Extend.x * std::abs(CutPlane.x) +
              Bounding.Extend.y * std::abs(CutPlane.y) +
              Bounding.Extend.z * std::abs(CutPlane.z);

    float Temp = GetDistToPlane(CutPlane, Bounding.Center);
  
    return -R <= Temp;
}

glm::mat4 ORMath::GetDirectionalViewProj(DirectionalLight& Light, glm::vec3 CamLocation, float UpDist, float Far,
    float ExtendH, float ExtendV)
{
    glm::vec3 PreSetLocation = CamLocation + Light.DirectionL * UpDist;
    glm::vec3 FocalPoint = PreSetLocation - Light.DirectionL;

    glm::mat4 View = glm::lookAt(PreSetLocation, FocalPoint, glm::vec3(0.0f, 1.0f, 0.0f));
    return View * glm::ortho(-ExtendH, ExtendH, -ExtendV, ExtendV, 0.0f, Far);
}

glm::mat4 ORMath::Transform2Matrix(const Transform& FromTransform)
{
    glm::mat4 ModelMatrix = glm::mat4_cast(FromTransform.Rotation) * glm::scale(glm::mat4(1.0), FromTransform.Scale);
    ModelMatrix[3] = glm::vec4(FromTransform.Translate, 1.0);

    return ModelMatrix;
}

float ORMath::GetDistToPlane(const glm::vec4& Plane, const glm::vec3& Point)
{
    return glm::dot(glm::vec3(Plane), Point) - Plane.w;
}

Transform ORMath::GetIdentityTransform()
{
    return Transform {
        glm::vec3(0.0f, 0.0f ,0.0f),
        glm::quat(),
        glm::vec3(1.0f,1.0f,1.0f)
    };
}

