#pragma once

#include <string>

#include "RHIEnums.h"
#include "RHIResource.h"

struct TextureDesc
{
    std::string TextureDebugName;
    TextureType Type = TextureType::Texture2D;
    uint32_t Wdith = 0;
    uint32_t Height = 0;

    uint32_t ArrayNum = 0;
    uint32_t MipmapLevel = 0;

    uint32_t SampleBit = 1;
    TextureUsage Usage = TextureUsage::None;
    RHIFormat TexelFormat = RHIFormat::UNDEFINED;
};

struct TextureSubresource
{
    inline constexpr static uint32_t AllLayer = UINT32_MAX;
    inline constexpr static uint32_t AllMipmap = UINT32_MAX;

    TexturePlaneBit Plane = TexturePlaneBit::None;
    uint32_t BaseArrayLayer = 0;
    uint32_t LayerCount = 0;
    uint32_t BaseMipmap = 0;
    uint32_t MipmapCount = 0;
};

class RHITexture : public RHIResource
{
    TextureDesc Desc;

public:

    virtual ~RHITexture() = default;
    
    const TextureDesc& GetDesc() const;
    
};


