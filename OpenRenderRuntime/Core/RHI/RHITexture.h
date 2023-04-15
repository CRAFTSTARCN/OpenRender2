#pragma once
#include "OpenRenderRuntime/Core/RHI/RHITexImage.h"
#include "OpenRenderRuntime/Core/RHI/RHITextureView.h"

struct TextureSamplerCreateStruct
{
    TextureWrap WrapU = TextureWrap_Repeat;
    TextureWrap WrapV = TextureWrap_Repeat;
    TextureWrap WrapW = TextureWrap_Repeat;
    
    FilterType FilterMag = FilterType_Linear;
    FilterType FilterMin = FilterType_Linear;

    MipmapFilterType MipmapFilter = MipmapFilterType_Nearest;
    uint32_t Anisotropy = 0;
    uint8_t BorderColor[4] = {0, 0, 0, 255};
};

struct TextureSubresource
{
    TexturePlaneBit Plane = TexturePlaneBit_Color;
    uint32_t FromLayer = 0;
    uint32_t LayerCount = 1;
    uint32_t FromMipmap = 0;
    uint32_t MipmapCount = 1;
};

struct TextureInfo
{
    uint32_t Width = 0;
    uint32_t Height = 0;
    RHIFormat Format = RHIFormat_RGBA8;
    TextureType TextureCreateType = TextureType_2D;
    TextureUsage Usage = TextureUsageBit_Transfer_Src | TextureUsageBit_Transfer_Dst | TextureUsageBit_Sample;
    uint32_t MipmapLevelCount = 1;
    uint32_t LayerCount = 1;
};

class RHITexture
{
    
public:
    TextureInfo TextureInfo {};
    
    RHITextureView* DefaultTextureView = nullptr;
};
