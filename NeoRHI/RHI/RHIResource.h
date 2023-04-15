#pragma once

class RHIResource
{

public:
    
    RHIResource() = default;

    RHIResource(const RHIResource&) = delete;
    RHIResource(RHIResource&&) = delete;
    
    virtual ~RHIResource() = default;

    RHIResource& operator=(const RHIResource&) = delete;
    RHIResource& operator=(RHIResource&&) = delete;
};
