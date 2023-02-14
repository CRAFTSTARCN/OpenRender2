#pragma once

#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

struct Transform
{
	glm::vec3 Translate {};
	glm::quat Rotation {};
	glm::vec3 Scale {};
};
