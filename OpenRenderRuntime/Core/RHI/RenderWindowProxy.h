#pragma once

#define GLFW_INCLUDE_VULKAN
#include <functional>

#include "GLFW/glfw3.h"

struct WindowSize
{
    int Width = 0;
    int Height = 0;
};

class RenderWindowProxy
{
    GLFWwindow* GLFWWindowHandle;

    
    /*
     * Number: 0 left 1 right 2 middle
     */
    std::function<void(uint32_t, bool)> MouseButtonFunction = nullptr;
    std::function<void(double X, double Y)> MousePosFunction = nullptr;
    std::function<void(uint32_t, bool)> KeyFunction = nullptr;
    std::function<void(uint32_t Width, uint32_t Height)> ResizeFunction = nullptr;

    static void InternalKeyFunc(GLFWwindow* Window, int Key, int Scancode, int Action, int Mods);
    static void InternalMouseButtonFunc(GLFWwindow* Window, int Button, int Action, int Mods);
    static void InternalMousePositionFunction(GLFWwindow* Window, double X, double Y);
    static void InternalWindowResizeFunction(GLFWwindow* Window, int Width, int Height);
    
public:

    RenderWindowProxy(GLFWwindow* Window);
    ~RenderWindowProxy();

    RenderWindowProxy(const RenderWindowProxy&) = delete;
    RenderWindowProxy(RenderWindowProxy&&) = delete;
    RenderWindowProxy& operator=(const RenderWindowProxy&) = delete;
    RenderWindowProxy& operator=(RenderWindowProxy&&) = delete;

    WindowSize GetWindowSize() const;
    
    bool WindowShouldClose();
    void ProcessSystemIO();
    
    void SetMouseFunction(std::function<void(uint32_t, bool)>&& InMouseButtonFunction);
    void SetMousePosFunction(std::function<void(double X, double Y)>&& InMousePosFunction);
    void SetKeyFunction(std::function<void(uint32_t, bool)>&& InKeyFunction);
    void SetResizeFunction(std::function<void(uint32_t Width, uint32_t Height)>&& InResizeFunction);
};
