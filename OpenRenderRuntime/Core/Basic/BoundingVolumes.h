#pragma once

#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include "glm/glm.hpp"
#include "OpenRenderRuntime/Util/Logger.h"

struct AABB
{
    glm::vec3 Center;
    glm::vec3 Extend;

    AABB() : Center(0,0,0), Extend(0,0,0) {}

    AABB(const glm::vec3& Min, const glm::vec3& Max)
    {
        if(Min.x > Max.x || Min.y > Max.y || Min.z > Max.z)
        {
            LOG_ERROR("Min conner may greater than max conner when construct AABB");
        }
        Center = (Min + Max) * 0.5f;
        Extend = (Max - Min) * 0.5f;
    }

    AABB(const AABB& In) : Center(In.Center), Extend(In.Extend)
    {
    }

    AABB& operator=(const AABB& In)
    {
        Center = In.Center;
        Extend = In.Extend;
        return *this;
    }

    glm::vec3 GetMinConner() const
    {
        return Center - Extend;
    }

    glm::vec3 GetMaxConner() const
    {
        return Center + Extend;
    }
    
};
