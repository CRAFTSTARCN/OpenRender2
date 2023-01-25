#include "OpenRenderRuntime/Modules/RenderConfig/RenderCofig.h"

RenderConfig::RenderConfig()
{
}


RenderConfig& RenderConfig::Get()
{
    static RenderConfig GlobalRenderConfig;
    return GlobalRenderConfig;
}
