#pragma once
#include "OpenRenderRuntime/Core/RHI/RHISemaphore.h"

//Waiter should hold a view of semaphore, not 
class RHISemaphoreView
{
	
public:
	RHISemaphore* BaseSemaphore;
	PipelineStage WaitStage;
};
