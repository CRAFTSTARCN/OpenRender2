#pragma once
#include "OpenRenderRuntime/Core/RHI/RHISampler.h"
#include "OpenRenderRuntime/Core/RHI/RHITexture.h"
#include "OpenRenderRuntime/Util/AutoIncreaseIdAllocator.h"

class RenderTexture
{
public:

    inline static AutoIncreaseIdAllocator Registry {0};
    constexpr static size_t BAD_TEXTURE_ID = SIZE_MAX;
    
    RHITexture* InternalTexture = nullptr;    
    RHISampler* Sampler = nullptr;
};
