#pragma once
#include <string>

#include "RHIEnums.h"

struct ShaderDesc
{
    std::string NamePath;
    std::string Entry = "main";
    ShaderStage ShaderStage = ShaderStage::None;
};

class RHIShader
{
    ShaderDesc Desc;

public:

    virtual ~RHIShader() = default;

    const ShaderDesc& GetDesc() const {return Desc;}
};