#pragma once
#include <vector>

#include "RHIBuffer.h"
#include "RHIEnums.h"
#include "RHIResource.h"
#include "RHITexture.h"

struct DescriptorBindingDesc
{
    DescriptorType Type;
    uint32_t BindingSlot;
    uint32_t BindingCount;
};

struct DescriptorLayoutDesc
{
    std::vector<DescriptorBindingDesc> Bindings;
    ShaderStage UsedByStage = ShaderStage::None;
    bool IsBindless = false;
};


class RHIDescriptorLayout
{
    DescriptorLayoutDesc Desc;

public:

    virtual ~RHIDescriptorLayout();
    
    const DescriptorLayoutDesc& GetDesc() const;
};

struct DescriptorItemDesc
{
    uint32_t Slot;
    uint32_t Index; //Valid for bindless
    RHIResource* Resource;

    union 
    {
        RHIFormat Format;
        uint32_t  StructureStride;
    };

    union 
    {
        TextureSubresource TextureSubresource;
        BufferRange Range;
    };

    DescriptorItemDesc()
    {
        Slot = 0;
        Index = 0;
        Resource = nullptr;
        Format = RHIFormat::UNDEFINED;
        StructureStride = 0;
        TextureSubresource  = {};
        Range = {};
    }
};

class RHIDescriptorSet
{

    DescriptorLayoutDesc LayoutDesc;

public:

    virtual ~RHIDescriptorSet();

    const DescriptorLayoutDesc& GetLayoutDesc() const;
    
};