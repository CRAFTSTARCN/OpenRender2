#pragma once
#include "RHIEnums.h"
#include "RHIResource.h"

struct BufferDesc
{
    size_t Size = 0;
    BufferUsage Usage = BufferUsage::None;
    MemoryFlag BufferMemory = MemoryFlag::None;
};

struct BufferRange
{
    size_t Offset = 0;
    size_t Size = 0;
};

class RHIBuffer : public RHIResource
{

    BufferDesc Desc;

public:

    virtual ~RHIBuffer() override = default;

    const BufferDesc& GetDesc() const;
    
};
