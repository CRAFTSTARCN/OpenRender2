#pragma once

#include <functional>
#include <vector>

#include "VulkanRHI.h"
#include "OpenRenderRuntime/Core/VulkanRHI/VulkanRHIContext.h"

class VKTool
{
    
public:

    template <typename FunctionType>
    inline static FunctionType LoadFunction(VulkanRHIContext* Context, const char* FunctionName)
    {
        return (FunctionType)vkGetInstanceProcAddr(Context->Instance, FunctionName);
    }

    template <typename FunctionType>
    inline static FunctionType LoadDeviceFunction(VulkanRHIContext* Context, const char* FunctionName)
    {
        return (FunctionType)vkGetDeviceProcAddr(Context->Device, FunctionName);
    }

    template <typename T,typename F>
    inline static std::vector<T> EnumAndGetAttrib(F EnumrateFunction, VkResult& OutResult)
    {
        uint32_t Count = 0;
        
        OutResult = EnumrateFunction(&Count, nullptr);

        std::vector<T> Result(Count);
        if(OutResult != VK_SUCCESS && Count != 0)
        {
            return Result;
        }

        OutResult = EnumrateFunction(&Count, Result.data());
        return Result;
    }

    template <typename T>
    inline static bool CheckSupport(
        const std::vector<const char *>& Required,
        const std::vector<T>&  Available, const std::function<bool(const char*, const T&)>& CmpFunction)
    {
        for (auto Need : Required) {
            bool Finded = false;

            for (const auto &Element : Available)
            {
                if(CmpFunction(Need, Element))
                {
                    Finded = true;
                    break;
                }
            }

            if(!Finded)
            {
                return false;
            }
        }

        return true;
    }
    template<typename T, typename B, typename F>
    inline static std::vector<T> EnumAndGetFrom(B From, F EnumrateFunction, VkResult& OutResult)
    {
        uint32_t Count = 0;
        OutResult = EnumrateFunction(From, &Count, nullptr);

        std::vector<T> Result(Count);
        if(OutResult != VK_SUCCESS && Count != 0)
        {
            return Result;
        }

        OutResult = EnumrateFunction(From, &Count, Result.data());
        return Result;
    }

    template <typename T, typename B1, typename B2, typename F>
    inline static std::vector<T> EnumAndGetFrom2P(B1 From1, B2 From2, F EnumrateFunction, VkResult& OutResult)
    {
        uint32_t Count = 0;
        OutResult = EnumrateFunction(From1, From2, &Count, nullptr);

        std::vector<T> Result(Count);
        if(OutResult != VK_SUCCESS && Count != 0)
        {
            return Result;
        }

        OutResult = EnumrateFunction(From1, From2, &Count, Result.data());
        return Result;
    }

    template <typename T, typename B1, typename B2, typename F>
    inline static std::vector<T> GetPropertiesArray2P(B1 From1, B2 From2, F GetFunction)
    {
        uint32_t Count = 0;
        GetFunction(From1, From2, &Count, nullptr);

        std::vector<T> Result(Count);
        GetFunction(From1, From2, &Count, Result.data());

        return Result;
    }

    template <typename T, typename B, typename F>
    inline static std::vector<T> GetPropertiesArrayP(B From, F GetFunction)
    {
        uint32_t Count = 0;
        GetFunction(From, &Count, nullptr);

        std::vector<T> Result(Count);
        GetFunction(From, &Count, Result.data());

        return Result;
    }

    static VKAPI_ATTR VkBool32 VKAPI_CALL ValidationLayerDebugCallBack(
    VkDebugUtilsMessageSeverityFlagBitsEXT MessageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT MessageType,
    const VkDebugUtilsMessengerCallbackDataEXT* CallbackData,
    void* UserData);

    static size_t ScoreGPU(VkPhysicalDevice PhysicalDevice);

    static std::vector<std::pair<size_t, VkPhysicalDevice>> GetRankedGPU(VulkanRHIContext* VulkanContext);

    static int FindQueueFamilyByBit(VkPhysicalDevice PhysicalDevice, VkQueueFlagBits Flag);

    static int FindPresentQueueFamily(VkPhysicalDevice PhysicalDevice, VulkanRHIContext* Context);

    static uint32_t FindMemoryIndex(VulkanRHIContext* Context, uint32_t MemoryType, VkMemoryPropertyFlags MemoryProperty);

    static VkShaderModule CreateShaderModuleInternal(VulkanRHIContext* Context, const void* Code, size_t CodeSize);
    
    /*
     * Create image
     */
    static bool CreateImage2D(
        VulkanRHIContext* Context,
        uint32_t Width,
        uint32_t Height,
        VkFormat ImageFormat,
        VkImageTiling ImageTilling,
        VkImageUsageFlags ImageUsageFlag,
        VkMemoryPropertyFlags MemoryProperty,
        VkImage& OutImage,
        VmaAllocation& OutAllocation,
        VkImageCreateFlags ImageCreateFlag,
        uint32_t LayerCount = 1,
        uint32_t MipmapLevelCount = 1,
        bool Share = false);

    /*
     * Create Image View
     */
    static VkImageView CreateImageView(
        VulkanRHIContext* Context,
        VkImage& Image,
        VkFormat Format,
        VkImageAspectFlags AspectFlag,
        VkImageViewType ImageViewType,
        uint32_t LayoutCount = 1,
        uint32_t MipmapCount = 1,
        uint32_t LayerBias = 0,
        uint32_t MipmapBias = 0);

    /*
     * Create textures with data, out layout is VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL
     * Load only first level of mipmap
     */
    static bool CreateTextureImage2D(
        VulkanRHI* RHI,
        VulkanRHIContext* Context,
        uint32_t Width,
        uint32_t Height,
        uint32_t PixelByteSize,
        uint32_t LayerCount,
        VkFormat Format,
        VkImageCreateFlags ImageCreateFlags, //For cubemap or normal texture
        uint32_t MipmapCount,
        const std::vector<void*>& Data,
        VkImage& OutImage,
        VmaAllocation& OutAllocation);

    /*
     * Create a buffer for any usage
     * Even staging buffer
     */
    static bool CreateBuffer(
        VulkanRHIContext* Context,
        VkDeviceSize BufferSize,
        VkBufferUsageFlags BufferUsage,
        VmaMemoryUsage MemoryUsage,
        VkBuffer& OutBuffer,
        VmaAllocation& OutAllocation,
        bool Share = false);

    /*
     * Creating a staging buffer
     */
    static bool CreateStagingBuffer(
        VulkanRHIContext* Context,
        VkDeviceSize Size,
        VkBuffer& OutStageBuffer,
        VkDeviceMemory& OutMemory);

    /*
     * Helper function to create pipeline layout
     */
    static VkPipelineLayout CreatePipelineLayout(
        VulkanRHIContext* Context,
        const std::vector<VkDescriptorSetLayout>& Layouts);

    /*
     * Helper function to create shader stage info
     */
    static VkPipelineShaderStageCreateInfo CreateShaderStageInfo(VkShaderModule Module, VkShaderStageFlagBits ShaderStage);

    /*
     * Helper function to create rasterization state
     */
    static VkPipelineRasterizationStateCreateInfo CreateRasterizationInfo(VkPolygonMode PolyFillMode, VkCullModeFlags CullMode, VkFrontFace FrontFaceSpec, float LineWidth);

    /*
     * Create attachment color blending
     */
    static VkPipelineColorBlendAttachmentState CreateBlendAttachment(bool Enable);

    /*
     * Create color blending stage
     */
    static VkPipelineColorBlendStateCreateInfo CreateColorBlendStageInfo(
        bool EnableLogic,
        VkLogicOp LogicOperation,
        const std::vector<VkPipelineColorBlendAttachmentState>& AttachmentBlend);

    /*
     * Create depth stencil stage info
     */
    static VkPipelineDepthStencilStateCreateInfo CreateDepthStencilStageInfo(
        bool EnableDepth,
        bool EnableDepthWrite,
        bool EnableStencil,
        VkCompareOp DepthCompareOp);

    /*
     * Create stencil test operation
     */
    static VkStencilOpState CreateStencilOp(
        VkCompareOp StencilCompareOp,
        uint32_t StencilCompareMask,
        uint32_t StencilWriteMask,
        uint32_t StencilWriteValue,
        VkStencilOp OnPass,
        VkStencilOp OnFail,
        VkStencilOp OnDepthFail);

    /*
     * Descriptor set write info
     */
    static VkWriteDescriptorSet CreateDescriptorWrite(VkDescriptorSet WriteSet, uint32_t Binding, VkDescriptorType Type);
    
    /*
     * Loading mipmap data for created texture
     */
    static void LoadMipmapDataImage2D(
        VkCommandBuffer CmdBuffer,
        VulkanRHIContext* Context,
        VkImage ToImage,
        uint32_t Width,
        uint32_t Height,
        uint32_t PixelByteSize,
        VkImageAspectFlags AspectFlag,
        uint32_t LayerCount,
        uint32_t MipmapLevel,
        const std::vector<void*>& Data,
        uint32_t LayerBias,
        VkBuffer& OutStagingBuffer,
        VkDeviceMemory& OutStagingBufferMemory);

    /*
     * Sampler create
     */
    static VkSampler CreateVulkanSampler(
        VulkanRHIContext* Context,
        VkSamplerAddressMode WrapU,
        VkSamplerAddressMode WrapV,
        VkSamplerAddressMode WrapW,
        VkFilter MinFilter,
        VkFilter MagFilter,
        VkSamplerMipmapMode MipmapFilter,
        uint32_t MipmapLevel,
        uint32_t Anisotropy,
        uint32_t BorderColor);

    /*
     * Generate new mipmap level
     * Source level require layout:
     * Source level out layout:
     * Generated level require layout:
     * Generated level layout: 
     */
    static void GenerateMipmapImage2D(
        VkCommandBuffer CmdBuffer,
        VulkanRHIContext* Context,
        VkImage ToImage,
        uint32_t OriWidth,
        uint32_t OriHeight,
        VkImageAspectFlags AspectFlag,
        uint32_t LayerCount,
        uint32_t MipmapLevel,
        VkFormat InternalFormat,
        uint32_t LayerBias = 0);

    /*
     * From Sascha Willems's Vulkan Examples
     * https://github.com/SaschaWillems/Vulkan
     */
    static void SetImageLayout(
        VkCommandBuffer CmdBuffer,
        VkImage Image,
        VkImageLayout OldImageLayout,
        VkImageLayout NewImageLayout,
        VkImageAspectFlags AspectFlag,
        VkPipelineStageFlags SrcStageMask,
        VkPipelineStageFlags DstStageMask,
        uint32_t LayerCount,
        uint32_t MipmapCount,
        uint32_t LayerBias = 0,
        uint32_t MipmapBias = 0);

    /*
     * Copy buffer to image
     */
    static void CopyBuffer2Image2D(
        VkCommandBuffer CmdBuffer,
        VkBuffer Buffer,
        VkImage Image,
        uint32_t Width,
        uint32_t Height,
        VkImageAspectFlags AspectFlags,
        uint32_t LayerCount,
        uint32_t MipmapLevel,
        uint32_t LayerBias = 0,
        uint32_t BufferOffset = 0,
        int32_t ImageOffsetU = 0,
        int32_t ImageOffsetV = 0);

    /*
     * Copy buffer to buffer
     */
    static void CopyBuffer2Buffer(
        VkCommandBuffer CmdBuffer,
        VkBuffer SrcBuffer,
        VkBuffer DstBuffer,
        VkDeviceSize Size,
        VkDeviceSize SrcOffset,
        VkDeviceSize DstOffset);

    /*
     * Copy image to image
     */
    static void CopyImage2Image2D(
        VkCommandBuffer CmdBuffer,
        VkImage Src,
        VkImage Dst,
        uint32_t Width,
        uint32_t Height,
        VkImageAspectFlags Aspect,
        uint32_t LayerCount,
        int32_t SrcImageOffsetU = 0,
        int32_t SrcImageOffsetV = 0,
        int32_t DstImageOffsetU = 0,
        int32_t DstImageOffsetV = 0,
        uint32_t SrcMipmapLevel = 0,
        uint32_t DstMipmapLevel = 0,
        uint32_t SrcLayerBias = 0,
        uint32_t DstLayerBias = 0);
};
