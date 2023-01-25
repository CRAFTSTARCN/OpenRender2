#pragma once

#include "OpenRenderRuntime/Core/RHI/RHIEnums.h"

class RHITexImage
{
    
public:
    uint32_t Width = 0;
    uint32_t Height = 0;
    uint32_t TotalLayer = 0;
    uint32_t TotalMipmap = 0;

    TexturePixelFormat Format = TexturePixelFormat_RGBA8;
};
