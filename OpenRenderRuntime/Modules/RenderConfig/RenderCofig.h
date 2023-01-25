#pragma once
#include <string>

class RenderConfig
{
    RenderConfig();
    
public:

    static RenderConfig& Get();

    //Only initial valid!!!!!!
    int RenderWindowWidth = 1920;
    int RenderWindowHeight = 1080;

    uint32_t ShadowMapResolutionWidth = 2048;
    uint32_t ShadowMapResolutionHeight = 2048;
    
    std::string RenderWindowName = "Open Render 2";

    std::string RootDir = "";
    
    std::string PipelineShaderDir = "./PassShader/";
    std::string RHIPlatform = "VulkanRHI";

    size_t GlobalDCBufferSize = 1 << 22;
    
    //VulkanRHI Setting
    bool VKEnableValidationLayer = true;
};
