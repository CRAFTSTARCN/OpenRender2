#pragma once

#include "OpenRenderRuntime/Core/RHI/RHIEnums.h"

class RHICommandList
{
    
public:
    RenderingTaskQueue CommandListQueue = RenderingTaskQueue_Graphics;
};