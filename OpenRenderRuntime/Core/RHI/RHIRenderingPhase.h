#pragma once

#include "OpenRenderRuntime/Core/RHI/RHIEnums.h"

class RHIRenderingPhase
{
    
public:
    RenderingTaskQueue PhaseQueue = RenderingTaskQueue_Graphics;
};