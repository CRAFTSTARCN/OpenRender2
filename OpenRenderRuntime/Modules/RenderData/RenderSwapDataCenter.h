#pragma once
#include <vector>

#include "OpenRenderRuntime/Modules/RenderData/RenderSwapData.h"

class RenderSwapDataCenter
{
	std::vector<RenderSwapData*> Front {};
	std::vector<RenderSwapData*> Back {};

	std::vector<RenderSwapData*>* LogicSide = nullptr;
	std::vector<RenderSwapData*>* RenderSide = nullptr;

public:

	RenderSwapDataCenter();
	~RenderSwapDataCenter();

	std::vector<RenderSwapData*>& GetLogicSide();
	std::vector<RenderSwapData*>& GetRenderSide();

	void EndLoopSwap();
};
