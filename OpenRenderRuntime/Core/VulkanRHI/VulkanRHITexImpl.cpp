#include "OpenRenderRuntime/Core/VulkanRHI/VulkanRHI.h"
#include "OpenRenderRuntime/Core/VulkanRHI/VKTool.h"
#include "OpenRenderRuntime/Util/Logger.h"

#include <cassert>

#include "OpenRenderRuntime/Core/VulkanRHI/VulkanRHITexImageView.h"
#include "OpenRenderRuntime/Core/VulkanRHI/VulkanRHITexImage.h"
#include "OpenRenderRuntime/Core/VulkanRHI/VulkanRHITexture.h"
#include "OpenRenderRuntime/Core/VulkanRHI/VulkanRHIBuffer.h"


RHITexture* VulkanRHI::CreateTexture2DManualMipmap(uint32_t Width, uint32_t Height, TexturePixelFormat Format,
                                                   const std::vector<std::vector<void*>>& Tex, TextureType TextureCreateType, ParamUsage Usage, uint32_t MipmapLevelCount,
                                                   uint32_t Anisotropy, uint32_t LayerCount, const TextureSamplerCreateStruct& SamplerInfo)
{
    VkImage Image = VK_NULL_HANDLE;
    VmaAllocation Allocation = VK_NULL_HANDLE;

    VkImageCreateFlags CreateFlag = 0;
    VkImageViewType ImageViewType = VK_IMAGE_VIEW_TYPE_2D;

    if(TextureCreateType == TextureType_Cube)
    {
        CreateFlag = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
        ImageViewType = VK_IMAGE_VIEW_TYPE_CUBE;

    }
    else if(TextureCreateType == TextureType_Array)
    {
        CreateFlag = VK_IMAGE_CREATE_2D_ARRAY_COMPATIBLE_BIT;
        ImageViewType = VK_IMAGE_VIEW_TYPE_2D_ARRAY;
    }
    
    bool Result = VKTool::CreateTextureImage2D(
        this,
        VulkanContext,
        Width,
        Height,
        FormatSizeTransfer[Format],
        LayerCount,
        FormatTransfer[Format],
        CreateFlag,
        MipmapLevelCount,
        Tex[0],
        Image,
        Allocation);

    if(!Result)
    {
        LOG_ERROR_FUNCTION("Fail to create texture");
        return nullptr;
    }

    VkCommandBuffer CmdBuffer = CreateSingletTimeCommandBuffer();

    if(MipmapLevelCount > 1)
    {
        VKTool::SetImageLayout(
            CmdBuffer,
            Image,
            VK_IMAGE_LAYOUT_UNDEFINED,
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            VK_IMAGE_ASPECT_COLOR_BIT,
            VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
            VK_PIPELINE_STAGE_TRANSFER_BIT,
            LayerCount,
            MipmapLevelCount - 1,
            0,
            1);
    }
    

    std::vector<VkBuffer> Buffers(MipmapLevelCount-1);
    std::vector<VkDeviceMemory> BufferMemory(MipmapLevelCount-1);
    for(uint32_t i = 1; i<MipmapLevelCount; ++i)
    {
        Width >>= 1;
        Height >>= 1;
        VKTool::LoadMipmapDataImage2D(
            CmdBuffer,
            VulkanContext,
            Image,
            Width,
            Height,
            FormatSizeTransfer[Format],
            VK_IMAGE_ASPECT_COLOR_BIT,
            LayerCount,
            i,
            Tex[i],
            0,
            Buffers[i-1],
            BufferMemory[i-1]);
    }

    VkPipelineStageFlags ToStage = 0;

    if(Usage & ParamUsageBit_Fragment)
    {
        ToStage |= VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    }
    if(Usage & ParamUsageBit_Geometry)
    {
        ToStage |= VK_PIPELINE_STAGE_VERTEX_SHADER_BIT | VK_PIPELINE_STAGE_MESH_SHADER_BIT_EXT; 
    }
    
    VKTool::SetImageLayout(
            CmdBuffer,
            Image,
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
            VK_IMAGE_ASPECT_COLOR_BIT,
            VK_PIPELINE_STAGE_TRANSFER_BIT,
            ToStage,
            LayerCount,
            MipmapLevelCount,
            0,
            0);

    EndSingleTimeCommandBuffer(CmdBuffer);
    for(size_t i=0 ; i<MipmapLevelCount - 1; ++i)
    {
        EndStageBuffer(Buffers[i], BufferMemory[i]);
    }

    VkSampler Sampler = GetSampler(MipmapLevelCount, SamplerInfo, Anisotropy);
    if(Sampler == VK_NULL_HANDLE)
    {
        LOG_ERROR("Cannot create sampler");
    }

    VkImageView DefaultView = VKTool::CreateImageView(
        VulkanContext,
        Image,
        FormatTransfer[Format],
        VK_IMAGE_ASPECT_COLOR_BIT,
        ImageViewType,
        LayerCount,
        MipmapLevelCount);

    VulkanRHITexImage* VKImage = new VulkanRHITexImage{{Width, Height, LayerCount, MipmapLevelCount, Format}, Image, Allocation};
    VulkanRHITexImageView* VKImageView = new VulkanRHITexImageView{{VKImage}, DefaultView};
    return new VulkanRHITexture{{VKImage, VKImageView}, Sampler};
}

RHITexture* VulkanRHI::CreateTexture2DAutoMipmap(uint32_t Width, uint32_t Height, TexturePixelFormat Format,
                                                 const std::vector<void*>& Tex, TextureType TextureCreateType, ParamUsage Usage, uint32_t MipmapLevelCount, uint32_t LayerCount,
                                                 uint32_t Anisotropy, const TextureSamplerCreateStruct& SamplerInfo)
{
    VkImage Image = VK_NULL_HANDLE;
    VmaAllocation Allocation = VK_NULL_HANDLE;

    VkImageCreateFlags CreateFlag = 0;
    VkImageViewType ImageViewType = VK_IMAGE_VIEW_TYPE_2D;

    if(TextureCreateType == TextureType_Cube)
    {
        CreateFlag = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
        ImageViewType = VK_IMAGE_VIEW_TYPE_CUBE;
    }
    else if(TextureCreateType == TextureType_Array)
    {
        CreateFlag = VK_IMAGE_CREATE_2D_ARRAY_COMPATIBLE_BIT;
        ImageViewType = VK_IMAGE_VIEW_TYPE_2D_ARRAY;

    }
    
    
    if(MipmapLevelCount == 0)
    {
        MipmapLevelCount = uint32_t(floor(std::log2(std::max(Width, Height)))) + 1u;
    }

    bool Result = VKTool::CreateTextureImage2D(
        this,
        VulkanContext,
        Width,
        Height,
        FormatSizeTransfer[Format],
        LayerCount,
        FormatTransfer[Format],
        CreateFlag,
        MipmapLevelCount,
        Tex,
        Image,
        Allocation);

    if(!Result)
    {
        LOG_ERROR_FUNCTION("Fail to create texture");
        return nullptr;
    }

    VkCommandBuffer CmdBuffer = CreateSingletTimeCommandBuffer();
    VKTool::SetImageLayout(
        CmdBuffer,
        Image,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
        VK_IMAGE_ASPECT_COLOR_BIT,
        VK_PIPELINE_STAGE_TRANSFER_BIT,
        VK_PIPELINE_STAGE_TRANSFER_BIT,
        LayerCount,
        1);

    uint32_t MipmapWidth = Width;
    uint32_t MipmapHeight = Height;
    for(uint32_t i=1; i<MipmapLevelCount; ++i)
    {
        VKTool::SetImageLayout(
            CmdBuffer,
            Image,
            VK_IMAGE_LAYOUT_UNDEFINED,
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            VK_IMAGE_ASPECT_COLOR_BIT,
            VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
            VK_PIPELINE_STAGE_TRANSFER_BIT,
            LayerCount,
            1,
            0,
            i);

        VKTool::GenerateMipmapImage2D(
            CmdBuffer,
            VulkanContext,
            Image,
            MipmapWidth,
            MipmapHeight,
            VK_IMAGE_ASPECT_COLOR_BIT,
            LayerCount,
            i,
            FormatTransfer[Format]);

        VKTool::SetImageLayout(
            CmdBuffer,
            Image,
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
            VK_IMAGE_ASPECT_COLOR_BIT,
            VK_PIPELINE_STAGE_TRANSFER_BIT,
            VK_PIPELINE_STAGE_TRANSFER_BIT,
            LayerCount,
            1,
            0,
            i);
        MipmapWidth >>= 1;
        MipmapHeight >>= 1;
    }

    VkPipelineStageFlags ToStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    if(Usage & ParamUsageBit_Geometry)
    {
        ToStage |= VK_PIPELINE_STAGE_MESH_SHADER_BIT_EXT | VK_PIPELINE_STAGE_VERTEX_SHADER_BIT;
    }
    
    VKTool::SetImageLayout(
        CmdBuffer,
        Image,
        VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
        VK_IMAGE_ASPECT_COLOR_BIT,
        VK_PIPELINE_STAGE_TRANSFER_BIT,
        ToStage,
        LayerCount,
        MipmapLevelCount);

    EndSingleTimeCommandBuffer(CmdBuffer);
    
    VkSampler Sampler = GetSampler(MipmapLevelCount, SamplerInfo, Anisotropy);
    if(Sampler == VK_NULL_HANDLE)
    {
        LOG_ERROR("Cannot create sampler");
    }

    VkImageView DefaultView = VKTool::CreateImageView(
        VulkanContext,
        Image,
        FormatTransfer[Format],
        VK_IMAGE_ASPECT_COLOR_BIT,
        ImageViewType,
        LayerCount,
        MipmapLevelCount);

    VulkanRHITexImage* VKImage = new VulkanRHITexImage{{Width, Height, LayerCount, MipmapLevelCount, Format}, Image, Allocation};
    VulkanRHITexImageView* VKImageView = new VulkanRHITexImageView{{VKImage}, DefaultView};
    return new VulkanRHITexture{{VKImage, VKImageView}, Sampler};
}


RHITexture* VulkanRHI::CreateTextureFromImageAttachment(RHIRenderImageAttachment* ImageAttachment, uint32_t LayerCount,
    uint32_t MipmapCount, uint32_t Layer, uint32_t MipmapLevel, TextureType TextureCreateType, uint32_t Anisotropy,
    const TextureSamplerCreateStruct& SampleInfo)
{
    RHITexImageView* ImageView = CreateImageViewFromImageAttachment(ImageAttachment, LayerCount, MipmapCount, Layer, MipmapLevel,  TextureCreateType);

    if(!ImageView)
    {
        return nullptr;
    }
    

    return new VulkanRHITexture{{nullptr, ImageView}, GetSampler(MipmapCount, SampleInfo, Anisotropy)};
}
