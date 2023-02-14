#pragma once
#include <json11.hpp>

#include "OpenRenderRuntime/Modules/AssetSystem/AssetObject.h"
#include "OpenRenderRuntime/Core/Basic/RenderMath.h"

using json11::Json;

struct SkyboxDataPath
{
	std::string Skybox;
	std::string Radiance;
	std::string Irradiance;
};

struct CameraData
{
	glm::vec3 Position;
	glm::vec3 Rotation;
	float Near = 0.0f;
	float Far = 100000.0f;
	float FOVV = 75.0f;

	float CameraMoveSpeed = 70.0f;
	float CameraLookSpeed = 0.1f;
};

struct LevelDirectionalLightData
{
	glm::vec4 Color;
	glm::vec3 Direction;
};

class LevelAsset : public AssetObject
{
	
public:

	std::vector<Json> ObjectJson;
	SkyboxDataPath SkyboxData;
	CameraData InitCameraData;
	LevelDirectionalLightData LightData;

	DEFINE_ASSET_TYPE("Level")
	
};
