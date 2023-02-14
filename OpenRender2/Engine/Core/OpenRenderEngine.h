#pragma once
#include "TickTimer.h"
#include "OpenRender2/Engine/Core/ViewportClient.h"
#include "OpenRender2/Engine/Core/LLevel.h"
#include "OpenRenderRuntime/Core/Render/Render.h"
#include "OpenRenderRuntime/Modules/AssetSystem/AssetSystem.h"
#include "OpenRenderRuntime/Modules/InputSystem/InputSystem.h"
#include "OpenRenderRuntime/Modules/RenderData/RenderSwapDataCenter.h"
#include "OpenRenderRuntime/Util/Barrier.h"

class OpenRenderEngine
{

	RenderSwapDataCenter* SwapDataCenter = nullptr;
	InputSystem* Input = nullptr;
	Render* RenderSystem = nullptr;
	AssetSystem* AssetSys = nullptr;
	ViewportClient* Client = nullptr;

	LLevel* CurrentLevel = nullptr;

	MultiTimeRunable RenderThread;
	Barrier LogicRenderBarrier{2};
	TickTimer Timer{};

	Json ProjectJson;
public:

	OpenRenderEngine();

	~OpenRenderEngine();

	InputSystem* GetInputSystem();

	RenderSwapDataCenter* GetSwapDataCenter();

	LLevel* GetCurrentLevel();

	AssetSystem* GetAssetSystem();

	ViewportClient* GetClient();

	void LoadProject(const std::string& ProjectFullPath);

	void OpenLevel(const std::string& LevelPath);

	void PreInit();

	void Init();

	void EngineLoop();

	void Terminate();
};

extern OpenRenderEngine* GEngine;