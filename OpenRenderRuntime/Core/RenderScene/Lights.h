#pragma once

#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

struct DirectionalLight
{
	glm::vec3 DirectionL {0.0f, 1.0f, 0.0f}; //Notice that the light direction means direction form object to light
	glm::vec3 Color {1.0f, 1.0f, 1.0f};
};
