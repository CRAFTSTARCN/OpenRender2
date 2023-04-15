#pragma once

class RHICommandList
{
    
public:

    virtual ~RHICommandList();

    virtual void Open() = 0;
    virtual void Close() = 0;
    
};
