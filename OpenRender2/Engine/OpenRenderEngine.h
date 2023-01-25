#pragma once
#include "OpenRenderRuntime/Core/Render/Render.h"
#include "OpenRenderRuntime/Modules/InputSystem/InputSystem.h"
#include "OpenRenderRuntime/Modules/RenderData/RenderSwapDataCenter.h"

class OpenRenderEngine
{

	RenderSwapDataCenter* SwapDataCenter;
	InputSystem* Input;
	Render* RenderSystem;
	
public:
	
};

extern OpenRenderEngine* GEngine;