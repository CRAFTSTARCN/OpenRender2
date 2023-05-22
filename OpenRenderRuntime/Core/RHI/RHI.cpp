#include "OpenRenderRuntime/Core/RHI/RHI.h"

RHI::RHI() :  WindowPtr(nullptr)
{
}

RHI::~RHI()
{
    
}

RenderWindowProxy& RHI::GetWindowProxy()
{
    static RenderWindowProxy GlobalWindowProxy(WindowPtr);
    return GlobalWindowProxy;
}

void RHI::SetResizeFunction(const std::function<void(uint32_t NewWidth, uint32_t NewHeight)>& Func)
{
    SwapchainRecreateFunction = Func;
}
