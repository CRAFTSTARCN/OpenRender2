#pragma once
#include <vector>

#include "OpenRender2/Engine/Core/GObject.h"
#include "OpenRenderRuntime/Modules/AssetSystem/AssetTypes/LevelAssets.h"
#include "OpenRenderRuntime/Modules/AssetSystem/AssetTypes/TextureAssets.h"

class LevelCamera
{

	CameraData Data;
	
	glm::vec3 Forward;
	glm::vec3 Right;

	bool DirtyLookPos = false;
	bool DirtyAll = false;

	uint32_t CaptureWidth = 0;
	uint32_t CaptureHeight = 0;
	
	void SendPosRotData2Swap();

	void SenFullData2Swap();
	
public:

	void Serialize(CameraData&& InCamData);
	void Begin();
	void Tick(float DeltaTime);
};


class LLevel
{
	std::vector<GObject*> Objects;
	LevelCamera LevelCamera {};
	LevelDirectionalLightData LightData;

	TextureCubeAsset* SkyboxAsset = nullptr;
	TextureCubeAsset* RadianceAsset = nullptr;
	TextureCubeAsset* IrradianceAsset = nullptr;

public:

	LLevel(size_t PreserveSpace = 1024);

	LLevel(const LLevel&) = delete;
	LLevel(LLevel&&) = delete;

	LLevel& operator=(const LLevel&) = delete;
	LLevel& operator=(LLevel&&) = delete;
	
	void Tick(float DeltaTime);
	
	void AddObject(GObject* Object);

	void SerializeFromAsset(LevelAsset* Asset);

	void Begin();

	void Unload();
	
};
