#pragma once
#include <string>
#include <unordered_map>

#include "OpenRenderRuntime/Core/RHI/RHISemaphore.h"

class RenderBlackboard
{
	
public:

	std::unordered_map<std::string, RHISemaphore*> RegisteredSemaphores {};
	
};
