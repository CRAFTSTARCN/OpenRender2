#pragma once
#include <string>
#include <unordered_map>

#include "OpenRenderRuntime/Core/RHI/RHISemaphore.h"
#include "OpenRenderRuntime/Util/Semaphore.h"

class RenderBlackboard
{
	
public:

	std::unordered_map<std::string, RHISemaphore*> RegisteredGPUSemaphores {};
	std::unordered_map<std::string, Semaphore*> RegisteredCPUSemaphores {};
	std::unordered_map<std::string, void*> AnyData {};
};
