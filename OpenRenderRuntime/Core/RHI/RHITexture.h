#pragma once
#include "OpenRenderRuntime/Core/RHI/RHITexImage.h"
#include "OpenRenderRuntime/Core/RHI/RHITexImageView.h"
#include "OpenRenderRuntime/Util/AutoIncreaseIdAllocator.h"

struct TextureSamplerCreateStruct
{
    TextureWrap WrapU = TextureWrap_Repeat;
    TextureWrap WrapV = TextureWrap_Repeat;
    TextureWrap WrapW = TextureWrap_Repeat;
    
    FilterType FilterMag = FilterType_Linear;
    FilterType FilterMin = FilterType_Linear;

    MipmapFilterType MipmapFilter = MipmapFilterType_Nearest;
    uint8_t BorderColor[4] = {0, 0, 0, 255};
};

class RHITexture
{
    
public:
    
    RHITexImage* TextureImage = nullptr;
    RHITexImageView* TextureImageView = nullptr;
};
