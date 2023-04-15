#pragma once

#include "OpenRenderRuntime/Core/RHI/RHIEnums.h"

class RHITexture;

class RHITextureView
{
    
public:

    RHITexture* ParentTexture = nullptr;
    RHIFormat ViewFormat = RHIFormat_None;
};
