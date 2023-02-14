#pragma once

#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

#include "RenderSwapData.h"

class CameraSetData : public RenderSwapData
{
	
public:

	glm::vec3 Position = glm::vec3(0.0f, 0.0f, 0.0f);
	glm::vec3 PitchYawRoll = glm::vec3(0.0f, 90.0f, 0.0f);

	float Near = 0.0f, Far = 10000.0f, FOV = 75.0f, Aspect = 1920.0f / 1080.0f;

	DEFINE_SWAP_DATA_TYPE(CameraSetData)
};

class CameraPositionLookData : public RenderSwapData
{
	public:
	glm::vec3 NewPosition = glm::vec3(0.0f, 0.0f, 0.0f);
	glm::vec3 NewRotation = glm::vec3(0.0, 0.0f, 0.0f);

	DEFINE_SWAP_DATA_TYPE(CameraPositionLookData)
};


class DirectionalLightData : public RenderSwapData
{
	public:

	bool UseAbsDir = true;

	
	glm::vec3 DirectionL {0.0f, 1.0f, 0.0f};
	glm::quat Rotation {};

	glm::vec3 Color {1.0f, 1.0f, 1.0f};

	DEFINE_SWAP_DATA_TYPE(DirectionalLightData)
	
};

/*
 * If not set, use BAD_TEXTURE_ID
 */
class EnvironmentTextureData : public RenderSwapData
{
	
public:

	size_t BRDFLUTId = 0;
	size_t SkyboxId = 0;
	size_t RadianceMapId = 0;
	size_t IrradianceMapId = 0;
	
	DEFINE_SWAP_DATA_TYPE(EnvironmentTextureData)
};
