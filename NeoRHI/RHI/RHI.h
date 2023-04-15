#pragma once
#include "RHIBuffer.h"
#include "RHIShader.h"
#include "RHITexture.h"

class RHI
{
    
public:

    RHI() = default;
    virtual ~RHI() = default;

    virtual RHIShader* CreateShader(const ShaderDesc* Desc, const void* Data, size_t Length);
    virtual RHITexture* CreateTexture(const TextureDesc& Desc) = 0;
    virtual RHIBuffer* CreateBuffer(const BufferDesc& Desc) = 0;

    
};
