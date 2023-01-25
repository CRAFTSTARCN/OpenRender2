#pragma once

#include "OpenRenderRuntime/Modules/RenderData/RenderSwapDataResolver.h"

class RenderSwapDataProcessor
{

protected:

	std::unordered_map<std::string, RenderSwapDataResolver*> Resolvers;
	std::vector<RenderSwapDataResolver*> ResolverCache;
	
public:

	
	virtual ~RenderSwapDataProcessor();
	
	virtual void Initialize(const RenderComponentsData& InData);

	virtual void ResolveData(RenderSwapData* Data);

	virtual void ProcessSwapData(const std::vector<RenderSwapData*>& SwapData);
	
	virtual void Terminate();
};
