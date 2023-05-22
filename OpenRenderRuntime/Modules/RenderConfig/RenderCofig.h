#pragma once
#include <string>

class RenderConfig
{
    RenderConfig();
    
public:

    RenderConfig(const RenderConfig&) = delete;
    RenderConfig(RenderConfig&&) = delete;

    RenderConfig& operator=(const RenderConfig&) = delete;
    RenderConfig& operator=(const RenderConfig&&) = delete;

    static RenderConfig& Get();

    //Only initial valid!!!!!!
    int RenderWindowWidth = 1920;
    int RenderWindowHeight = 1080;

    uint32_t ShadowMapResolutionWidth = 2048;
    uint32_t ShadowMapResolutionHeight = 2048;
    
    std::string RenderWindowName = "Open Render 2";

    //Long term valid

    std::string RootDir = "";
    
    std::string PipelineShaderDir = "./PassShader/";
    std::string RHIPlatform = "VulkanRHI";

    size_t GlobalDCBufferSize = 1 << 22;
    
    /*
     * Vulkan rhi setting
     */
	//Initial valid
    bool VKEnableValidationLayer = false;
};
