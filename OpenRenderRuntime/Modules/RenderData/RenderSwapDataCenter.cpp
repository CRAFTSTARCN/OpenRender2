#include "OpenRenderRuntime/Modules/RenderData/RenderSwapDataCenter.h"

#include <thread>

RenderSwapDataCenter::RenderSwapDataCenter()
{
	LogicSide = &Front;
	RenderSide = &Back;
}

RenderSwapDataCenter::~RenderSwapDataCenter() = default;

std::vector<RenderSwapData*>& RenderSwapDataCenter::GetLogicSide()
{
	return *LogicSide;
}

std::vector<RenderSwapData*>& RenderSwapDataCenter::GetRenderSide()
{
	return *RenderSide;
}

void RenderSwapDataCenter::EndLoopSwap()
{
	RenderSide->clear();
	std::swap(LogicSide, RenderSide);
}
