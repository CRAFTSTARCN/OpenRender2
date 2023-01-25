#pragma once

#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

struct Frustum
{
    glm::vec4 Top;
    glm::vec4 Down;
    glm::vec4 Left;
    glm::vec4 Right;
    glm::vec4 Near;
    glm::vec4 Far;
};
