#pragma once

#include "OpenRenderRuntime/Core/RHI/RHIEnums.h"

class RHICommandList
{
    
public:
    RenderingTaskQueue PhaseQueue = RenderingTaskQueue_Graphics;
};