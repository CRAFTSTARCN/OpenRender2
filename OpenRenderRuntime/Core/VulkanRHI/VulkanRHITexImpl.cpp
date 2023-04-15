#include "OpenRenderRuntime/Core/VulkanRHI/VulkanRHI.h"
#include "OpenRenderRuntime/Core/VulkanRHI/VKTool.h"
#include "OpenRenderRuntime/Util/Logger.h"

#include <cassert>

#include "OpenRenderRuntime/Core/VulkanRHI/VulkanRHISampler.h"
#include "OpenRenderRuntime/Core/VulkanRHI/VulkanRHITextureView.h"
#include "OpenRenderRuntime/Core/VulkanRHI/VulkanRHITexture.h"
#include "OpenRenderRuntime/Core/VulkanRHI/VulkanRHIBuffer.h"


RHITexture* VulkanRHI::CreateTexture2DManualMipmap(
        const TextureInfo& Info,
        const std::vector<std::vector<void*>>& Tex)
{
    VkImage Image = VK_NULL_HANDLE;
    VmaAllocation Allocation = VK_NULL_HANDLE;

    VkImageCreateFlags CreateFlag = 0;
    VkImageViewType ImageViewType = VK_IMAGE_VIEW_TYPE_2D;

    if(Info.TextureCreateType == TextureType_Cube)
    {
        CreateFlag = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
        ImageViewType = VK_IMAGE_VIEW_TYPE_CUBE;

    }
    else if(Info.TextureCreateType == TextureType_Array)
    {
        //CreateFlag = VK_IMAGE_CREATE_2D_ARRAY_COMPATIBLE_BIT;
        ImageViewType = VK_IMAGE_VIEW_TYPE_2D_ARRAY;
    }
    
    VkImageUsageFlags Usage = TransferImageUsage(Info.Usage);
    bool Result = VKTool::CreateTextureImage2D(
        this,
        VulkanContext,
        Info.Width,
        Info.Height,
        FormatSizeTransfer[Info.Format],
        Info.LayerCount,
        FormatTransfer[Info.Format],
        Usage,
        CreateFlag,
        Info.MipmapLevelCount,
        Tex[0],
        Image,
        Allocation);

    if(!Result)
    {
        LOG_ERROR_FUNCTION("Fail to create texture");
        return nullptr;
    }

    VkCommandBuffer CmdBuffer = CreateSingletTimeCommandBuffer();

    if(Info.MipmapLevelCount > 1)
    {
        VKTool::SetImageLayout(
            CmdBuffer,
            Image,
            VK_IMAGE_LAYOUT_UNDEFINED,
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            TransferAspectWithFormat(Info.Format),
            VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
            VK_PIPELINE_STAGE_TRANSFER_BIT,
            Info.LayerCount,
            Info.MipmapLevelCount - 1,
            0,
            1);
    }
    

    std::vector<VkBuffer> Buffers(Info.MipmapLevelCount-1);
    std::vector<VkDeviceMemory> BufferMemory(Info.MipmapLevelCount-1);
    uint32_t Width = Info.Width;
    uint32_t Height = Info.Height;
    for(uint32_t i = 1; i<Info.MipmapLevelCount; ++i)
    {
        Width >>= 1;
        Height >>= 1;
        VKTool::LoadMipmapDataImage2D(
            CmdBuffer,
            VulkanContext,
            Image,
            Width,
            Height,
            FormatSizeTransfer[Info.Format],
            TransferAspectWithFormat(Info.Format),
            Info.LayerCount,
            i,
            Tex[i],
            0,
            Buffers[i-1],
            BufferMemory[i-1]);
    }

    VkPipelineStageFlags ToStage =
        VK_PIPELINE_STAGE_VERTEX_SHADER_BIT | VK_PIPELINE_STAGE_MESH_SHADER_BIT_EXT | VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    
    
    VKTool::SetImageLayout(
            CmdBuffer,
            Image,
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
            TransferAspectWithFormat(Info.Format),
            VK_PIPELINE_STAGE_TRANSFER_BIT,
            ToStage,
            Info.LayerCount,
            Info.MipmapLevelCount,
            0,
            0);

    EndSingleTimeCommandBuffer(CmdBuffer);
    for(size_t i=0 ; i<Info.MipmapLevelCount - 1; ++i)
    {
        EndStageBuffer(Buffers[i], BufferMemory[i]);
    }
    
    VkImageView DefaultView = VKTool::CreateImageView(
        VulkanContext,
        Image,
        FormatTransfer[Info.Format],
        TransferAspectWithFormat(Info.Format),
        ImageViewType,
        Info.LayerCount,
        Info.MipmapLevelCount);

    VulkanRHITexture* VKTexture = new VulkanRHITexture{
        {Info, nullptr},
        Image,
        Allocation};
    VulkanRHITextureView* VKTextureView = new VulkanRHITextureView{
        {VKTexture, Info.Format},
        DefaultView};
    VKTexture->DefaultTextureView = VKTextureView;
    return VKTexture;
}

RHITexture* VulkanRHI::CreateTexture2DAutoMipmap(
    const TextureInfo& Info,
    const std::vector<void*>& Tex)
{
    VkImage Image = VK_NULL_HANDLE;
    VmaAllocation Allocation = VK_NULL_HANDLE;

    VkImageCreateFlags CreateFlag = 0;
    VkImageViewType ImageViewType = VK_IMAGE_VIEW_TYPE_2D;

    if(Info.TextureCreateType == TextureType_Cube)
    {
        CreateFlag = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
        ImageViewType = VK_IMAGE_VIEW_TYPE_CUBE;
    }
    else if(Info.TextureCreateType == TextureType_Array)
    {
        ImageViewType = VK_IMAGE_VIEW_TYPE_2D_ARRAY;

    }
    
    uint32_t MipmapLevelCount = Info.MipmapLevelCount;
    if(MipmapLevelCount == 0)
    {
        MipmapLevelCount = uint32_t(floor(std::log2(std::max(Info.Width, Info.Height)))) + 1u;
    }

    VkImageUsageFlags Usage = TransferImageUsage(Info.Usage);
    bool Result = VKTool::CreateTextureImage2D(
        this,
        VulkanContext,
        Info.Width,
        Info.Height,
        FormatSizeTransfer[Info.Format],
        Info.LayerCount,
        FormatTransfer[Info.Format],
        Usage,
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
        TransferAspectWithFormat(Info.Format),
        VK_PIPELINE_STAGE_TRANSFER_BIT,
        VK_PIPELINE_STAGE_TRANSFER_BIT,
        Info.LayerCount,
        1);

    uint32_t MipmapWidth = Info.Width;
    uint32_t MipmapHeight = Info.Height;
    for(uint32_t i=1; i<MipmapLevelCount; ++i)
    {
        VKTool::SetImageLayout(
            CmdBuffer,
            Image,
            VK_IMAGE_LAYOUT_UNDEFINED,
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            TransferAspectWithFormat(Info.Format),
            VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
            VK_PIPELINE_STAGE_TRANSFER_BIT,
            Info.LayerCount,
            1,
            0,
            i);

        VKTool::GenerateMipmapImage2D(
            CmdBuffer,
            VulkanContext,
            Image,
            MipmapWidth,
            MipmapHeight,
            TransferAspectWithFormat(Info.Format),
            Info.LayerCount,
            i,
            FormatTransfer[Info.Format]);

        VKTool::SetImageLayout(
            CmdBuffer,
            Image,
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
            TransferAspectWithFormat(Info.Format),
            VK_PIPELINE_STAGE_TRANSFER_BIT,
            VK_PIPELINE_STAGE_TRANSFER_BIT,
            Info.LayerCount,
            1,
            0,
            i);
        MipmapWidth >>= 1;
        MipmapHeight >>= 1;
    }

    VkPipelineStageFlags ToStage =
        VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_MESH_SHADER_BIT_EXT | VK_PIPELINE_STAGE_VERTEX_SHADER_BIT;
    
    
    VKTool::SetImageLayout(
        CmdBuffer,
        Image,
        VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
        TransferAspectWithFormat(Info.Format),
        VK_PIPELINE_STAGE_TRANSFER_BIT,
        ToStage,
        Info.LayerCount,
        MipmapLevelCount);

    EndSingleTimeCommandBuffer(CmdBuffer);

    VkImageView DefaultView = VKTool::CreateImageView(
        VulkanContext,
        Image,
        FormatTransfer[Info.Format],
        TransferAspectWithFormat(Info.Format),
        ImageViewType,
        Info.LayerCount,
        MipmapLevelCount);

    VulkanRHITexture* VKTexture = new  VulkanRHITexture{
            {Info, nullptr},
            Image,
            Allocation};
    
    VulkanRHITextureView* VKTextureView = new VulkanRHITextureView{
        {VKTexture, Info.Format},
        DefaultView};

    VKTexture->TextureInfo.MipmapLevelCount = MipmapLevelCount;
    VKTexture->DefaultTextureView = VKTextureView;
    return VKTexture;
}

RHITexture* VulkanRHI::CreateTexture2D(const TextureInfo& Info)
{
    VkImage Image = VK_NULL_HANDLE;
    VmaAllocation Allocation = VK_NULL_HANDLE;

    VkImageUsageFlags Usage = TransferImageUsage(Info.Usage);

    VkImageCreateFlags CreateFlag = 0;
    VkImageViewType CreateType = VK_IMAGE_VIEW_TYPE_2D;
    if(Info.LayerCount > 1)
    {
        CreateFlag = VK_IMAGE_CREATE_2D_ARRAY_COMPATIBLE_BIT;
        CreateType = VK_IMAGE_VIEW_TYPE_2D_ARRAY;
    }

    bool Result = VKTool::CreateImage2D(
        VulkanContext,
        Info.Width,
        Info.Height,
        FormatTransfer[Info.Format],
        VK_IMAGE_TILING_OPTIMAL,
        Usage,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        Image,
        Allocation,
        CreateFlag,
        Info.LayerCount,
        Info.MipmapLevelCount);
    

    if(!Result)
    {
        LOG_ERROR("Fail to create attachment image");
        return nullptr;
    }

    
    VkImageView DefaultView = VKTool::CreateImageView(
        VulkanContext,
        Image,
        FormatTransfer[Info.Format],
        TransferAspectWithFormat(Info.Format),
        CreateType,
        Info.LayerCount,
        Info.MipmapLevelCount);

    
    VulkanRHITexture* VKTexture = new  VulkanRHITexture{
                {Info, nullptr},
                Image,
                Allocation};
    
    VulkanRHITextureView* VKTextureView = new VulkanRHITextureView{
            {VKTexture, Info.Format},
            DefaultView};

    VKTexture->DefaultTextureView = VKTextureView;
    
    return VKTexture;
}

RHISampler* VulkanRHI::CreateOrGetSampler(const TextureSamplerCreateStruct& CreateInfo)
{
    return new VulkanRHISampler{{}, GetSampler(CreateInfo)};
}
