#include "RenderWindowProxy.h"

#include <cassert>

#include "OpenRenderRuntime/Util/Logger.h"

void RenderWindowProxy::InternalKeyFunc(GLFWwindow* Window, int Key, int Scancode, int Action, int Mods)
{
    RenderWindowProxy* WindowProxy = (RenderWindowProxy*)glfwGetWindowUserPointer(Window);

    if(!WindowProxy)
    {
        LOG_ERROR_FUNCTION("No proxy ptr for raw glfw window {0}", (void*)Window);
    }
    
    if(Action == GLFW_PRESS)
    {
        LOG_INFO("Key press {0}", Key);
        if(WindowProxy->KeyFunction)
        {
            WindowProxy->KeyFunction(Key, true);
        }
    }
    else if (Action == GLFW_RELEASE)
    {
        LOG_INFO("Key release {0}", Key);
        if(WindowProxy->KeyFunction)
        {
            WindowProxy->KeyFunction(Key, false);
        }
    }
}

void RenderWindowProxy::InternalMouseButtonFunc(GLFWwindow* Window, int Button, int Action, int Mods)
{
    RenderWindowProxy* WindowProxy = (RenderWindowProxy*)glfwGetWindowUserPointer(Window);

    if(!WindowProxy)
    {
        LOG_ERROR_FUNCTION("No proxy ptr for raw glfw window {0}", (void*)Window);
    }
    if(Action == GLFW_PRESS)
    {
        LOG_INFO("Mouse down {0}", Button);
        if(WindowProxy->KeyFunction)
        {
            WindowProxy->MouseButtonFunction(Button, true);
        }
    }
    else if(Action == GLFW_RELEASE)
    {
        LOG_INFO("Mouse up {0}", Button);
        if(WindowProxy->KeyFunction)
        {
            WindowProxy->MouseButtonFunction(Button, false);
        }
    }
}

void RenderWindowProxy::InternalMousePositionFunction(GLFWwindow* Window, double X, double Y)
{
    RenderWindowProxy* WindowProxy = (RenderWindowProxy*)glfwGetWindowUserPointer(Window);

    if(!WindowProxy)
    {
        LOG_ERROR_FUNCTION("No proxy ptr for raw glfw window {0}", (void*)Window);
    }
    //LOG_INFO("({0}, {1})", X, Y);
    if(WindowProxy->MousePosFunction)
    {
        WindowProxy->MousePosFunction(X, Y);
    }
}

void RenderWindowProxy::InternalWindowResizeFunction(GLFWwindow* Window, int Width, int Height)
{
    RenderWindowProxy* WindowProxy = (RenderWindowProxy*)glfwGetWindowUserPointer(Window);
    if(WindowProxy->ResizeFunction)
    {
        WindowProxy->ResizeFunction((uint32_t)Width, (uint32_t)Height);
    }
}

RenderWindowProxy::RenderWindowProxy(GLFWwindow* Window) :
GLFWWindowHandle(Window)
{
    assert(GLFWWindowHandle);
    if(glfwGetWindowUserPointer(Window) != nullptr)
    {
        LOG_ERROR_FUNCTION("Cannot create multi proxy for one window");
        assert(false);

        
    }

    glfwSetWindowUserPointer(Window, this);
    glfwSetKeyCallback(Window, RenderWindowProxy::InternalKeyFunc);
    glfwSetMouseButtonCallback(Window, RenderWindowProxy::InternalMouseButtonFunc);
    glfwSetCursorPosCallback(Window, RenderWindowProxy::InternalMousePositionFunction);
    glfwSetFramebufferSizeCallback(Window, RenderWindowProxy::InternalWindowResizeFunction);
}

RenderWindowProxy::~RenderWindowProxy()
{
}


WindowSize RenderWindowProxy::GetWindowSize() const
{
    WindowSize Size {};
    glfwGetWindowSize(GLFWWindowHandle, &Size.Width, &Size.Height);
    return Size;
}

bool RenderWindowProxy::WindowShouldClose()
{
    return glfwWindowShouldClose(GLFWWindowHandle);
}


void RenderWindowProxy::ProcessSystemIO()
{
    glfwPollEvents();
}

void RenderWindowProxy::SetMouseFunction(std::function<void(uint32_t, bool)>&& InMouseButtonFunction)
{
    MouseButtonFunction = std::move(InMouseButtonFunction);
}

void RenderWindowProxy::SetMousePosFunction(std::function<void(double X, double Y)>&& InMousePosFunction)
{
    MousePosFunction = std::move(InMousePosFunction);
}

void RenderWindowProxy::SetKeyFunction(std::function<void(uint32_t, bool)>&& InKeyFunction)
{
    KeyFunction = std::move(InKeyFunction);
}

void RenderWindowProxy::SetResizeFunction(std::function<void(uint32_t Width, uint32_t Height)>&& InResizeFunction)
{
    ResizeFunction = std::move(InResizeFunction);
}
