#include "OpenRenderRuntime/Core/VulkanRHI/VKTool.h"
#include "OpenRenderRuntime/Util/Logger.h"
#include "OpenRenderRuntime/Core/VulkanRHI/GPURankHelper.h"

VkBool32 VKTool::ValidationLayerDebugCallBack(
    VkDebugUtilsMessageSeverityFlagBitsEXT MessageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT MessageType,
    const VkDebugUtilsMessengerCallbackDataEXT* CallbackData,
    void* UserData)
{
	static std::vector<const char*> ShutErrorMessage {
		"VUID-VkGraphicsPipelineCreateInfo-stage-00727",
		"VUID-VkPipelineShaderStageCreateInfo-stage-02091"
	};
 
	for(auto Name : ShutErrorMessage)
	{
		if(strcmp(Name, CallbackData->pMessageIdName) == 0)
		{
			return VK_FALSE;
		}
	}
	
     if(MessageSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT) {
         LOG_ERROR_FUNCTION(CallbackData->pMessage);
     } else if(MessageSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) {
         LOG_WARN_FUNCTION(CallbackData->pMessage);
     } else if(MessageSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT) {
         LOG_INFO_FUNCTION(CallbackData->pMessage);
     }
    
     return VK_FALSE;
 }

size_t VKTool::ScoreGPU(VkPhysicalDevice PhysicalDevice)
{
    VkPhysicalDeviceProperties DeviceProperty;
    vkGetPhysicalDeviceProperties(PhysicalDevice, &DeviceProperty);

    size_t Score = 0;

    if(DeviceProperty.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
    {
        Score += 1000;  
    }
    else if(DeviceProperty.deviceType == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU)
    {
        Score += 100;
    } else
    {
        Score += 10;
    }

    VkPhysicalDeviceFeatures PhysicalDeviceFeatures;
    vkGetPhysicalDeviceFeatures(PhysicalDevice, &PhysicalDeviceFeatures);

    for(auto Feature : Features)
    {
        std::byte* Address = reinterpret_cast<std::byte*>(&PhysicalDeviceFeatures);
        if(*(reinterpret_cast<VkBool32*>(Address + Feature.Offset)) == VK_TRUE)
        {
            Score += Feature.Weight;
        }
        else
        {
            if(Feature.ForceRequired)
            {
                Score = 0;
                break;
            }
        }
    }

    return Score;
}

std::vector<std::pair<size_t, VkPhysicalDevice>> VKTool::GetRankedGPU(VulkanRHIContext* VulkanContext)
{
    VkResult PhysicalDeviceResult;
    LOG_DEBUG_FUNCTION("Querying physical device");
    std::vector<VkPhysicalDevice> PhysicalDevices = EnumAndGetFrom<VkPhysicalDevice>(
        VulkanContext->Instance,
        vkEnumeratePhysicalDevices,
        PhysicalDeviceResult);

    std::vector<std::pair<size_t, VkPhysicalDevice>> RankedGPUs;
    for(auto GPU : PhysicalDevices)
    {
        RankedGPUs.emplace_back(ScoreGPU(GPU), GPU);
    }

    return RankedGPUs;
}

int VKTool::FindQueueFamilyByBit(VkPhysicalDevice PhysicalDevice, VkQueueFlagBits Flag)
{
    int QueueFamily = -1;

    uint32_t QueueCount;
    vkGetPhysicalDeviceQueueFamilyProperties(PhysicalDevice, &QueueCount, nullptr);

    std::vector<VkQueueFamilyProperties> QueueFamilyPropties(QueueCount);
    vkGetPhysicalDeviceQueueFamilyProperties(PhysicalDevice, &QueueCount, QueueFamilyPropties.data());

    int i = 0;
    for (auto &Properties : QueueFamilyPropties) {
        if(Properties.queueFlags & Flag) {
            QueueFamily = i;
            break;
        }
        i++;
    }
    
    return QueueFamily;
}

int VKTool::FindPresentQueueFamily(VkPhysicalDevice PhysicalDevice, VulkanRHIContext* Context)
{
    int QueueFamily = -1;

    uint32_t QueueCount;
    vkGetPhysicalDeviceQueueFamilyProperties(PhysicalDevice, &QueueCount, nullptr);

    VkBool32 Presentable = false;
    for (uint32_t i=0; i<QueueCount; ++i) {
        VkResult FindResult = vkGetPhysicalDeviceSurfaceSupportKHR(PhysicalDevice, i, Context->WindowSurface, &Presentable);
        if(FindResult == VK_SUCCESS) {
            QueueFamily = (int)i;
            break;
        }
    }
    
    return QueueFamily;
}

uint32_t VKTool::FindMemoryIndex(VulkanRHIContext* Context, uint32_t MemoryType, VkMemoryPropertyFlags MemoryProperty)
{
    VkPhysicalDeviceMemoryProperties PhysicalDeviceMemoryProperty;
    vkGetPhysicalDeviceMemoryProperties(Context->PhysicalDevice, &PhysicalDeviceMemoryProperty);
    for (uint32_t i = 0; i < PhysicalDeviceMemoryProperty.memoryTypeCount; i++)
    {
        if (MemoryType & (1 << i) &&
            (PhysicalDeviceMemoryProperty.memoryTypes[i].propertyFlags & MemoryProperty) == MemoryProperty)
        {
            return i;
        }
    }

    return UINT_MAX;
}

VkShaderModule VKTool::CreateShaderModuleInternal(VulkanRHIContext* Context, const void* Code, size_t CodeSize)
{
	VkShaderModuleCreateInfo ShaderModuleInfo {};
	ShaderModuleInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	ShaderModuleInfo.codeSize = CodeSize;
	ShaderModuleInfo.pCode = (const uint32_t*)Code;

	VkShaderModule ShaderModule = VK_NULL_HANDLE;
	VkResult Result = vkCreateShaderModule(Context->Device, &ShaderModuleInfo, nullptr, &ShaderModule);
	if(Result != VK_SUCCESS)
	{
		LOG_ERROR_FUNCTION("Fail to create shader module");
		return VK_NULL_HANDLE;
	}

	return ShaderModule;
}

bool VKTool::CreateImage2D(VulkanRHIContext* Context, uint32_t Width, uint32_t Height, VkFormat ImageFormat,
                           VkImageTiling ImageTilling, VkImageUsageFlags ImageUsageFlag,
                           VkMemoryPropertyFlags MemoryProperty,
                           VkImage& OutImage, VmaAllocation& OutAllocation, VkImageCreateFlags ImageCreateFlag,
                           uint32_t LayerCount,
                           uint32_t MipmapLevelCount)
{
    VkImageCreateInfo ImageInfo {};
    ImageInfo.sType         = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    ImageInfo.flags         = ImageCreateFlag; 
    ImageInfo.imageType     = VK_IMAGE_TYPE_2D;
    ImageInfo.extent.width  = Width;
    ImageInfo.extent.height = Height;
    ImageInfo.extent.depth  = 1;
    ImageInfo.mipLevels     = MipmapLevelCount;
    ImageInfo.arrayLayers   = LayerCount;
    ImageInfo.format        = ImageFormat;
    ImageInfo.tiling        = ImageTilling;
    ImageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    ImageInfo.usage         = ImageUsageFlag;
    ImageInfo.samples       = VK_SAMPLE_COUNT_1_BIT;
	ImageInfo.sharingMode   = VK_SHARING_MODE_EXCLUSIVE;
	

    VkResult Result = vkCreateImage(Context->Device, &ImageInfo, nullptr, &OutImage);
    if(Result != VK_SUCCESS)
    {
        return false;
    }

    VkMemoryRequirements MemoryRequirements {};
    vkGetImageMemoryRequirements(Context->Device, OutImage, &MemoryRequirements);
    

    VmaAllocationCreateInfo AllocationInfo {};
    AllocationInfo.requiredFlags = MemoryProperty;
    AllocationInfo.memoryTypeBits = MemoryRequirements.memoryTypeBits;

    Result = vmaAllocateMemoryForImage(Context->Allocator, OutImage, &AllocationInfo, &OutAllocation, nullptr);
	vmaBindImageMemory(Context->Allocator, OutAllocation, OutImage);
	return Result == VK_SUCCESS;
}

VkImageView VKTool::CreateImageView(VulkanRHIContext* Context, VkImage& Image, VkFormat Format,
                                    VkImageAspectFlags AspectFlag, VkImageViewType ImageViewType, uint32_t LayoutCount, uint32_t MipmapCount,
                                    uint32_t LayerBias, uint32_t MipmapBias)
{
    VkImageViewCreateInfo ImageViewInfo {};
    ImageViewInfo.sType                           = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    ImageViewInfo.image                           = Image;
    ImageViewInfo.viewType                        = ImageViewType;
    ImageViewInfo.format                          = Format;
    ImageViewInfo.subresourceRange.aspectMask     = AspectFlag;
    ImageViewInfo.subresourceRange.baseMipLevel   = MipmapBias;
    ImageViewInfo.subresourceRange.levelCount     = MipmapCount;
    ImageViewInfo.subresourceRange.baseArrayLayer = LayerBias;
    ImageViewInfo.subresourceRange.layerCount     = LayoutCount;

    VkImageView ImageView = VK_NULL_HANDLE;
    if (vkCreateImageView(Context->Device, &ImageViewInfo, nullptr, &ImageView) != VK_SUCCESS)
    {
        return VK_NULL_HANDLE;
    }

    return ImageView;
}

bool VKTool::CreateTextureImage2D(VulkanRHI* RHI, VulkanRHIContext* Context, uint32_t Width, uint32_t Height,
                                  uint32_t PixelByteSize, uint32_t LayerCount, VkFormat Format, VkImageUsageFlags Usage,
                                  VkImageCreateFlags ImageCreateFlags, uint32_t MipmapCount,
                                  const std::vector<void*>& Data, VkImage& OutImage, VmaAllocation& OutAllocation)
{
    if(LayerCount != Data.size())
    {
        LOG_ERROR_FUNCTION("Layer count doesn't fit layer count");
    }
    VkImageCreateInfo ImageInfo {};
    ImageInfo.sType         = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    ImageInfo.flags         = ImageCreateFlags;
    ImageInfo.imageType     = VK_IMAGE_TYPE_2D;
    ImageInfo.extent.width  = Width;
    ImageInfo.extent.height = Height;
    ImageInfo.extent.depth  = 1;
    ImageInfo.mipLevels     = MipmapCount;
    ImageInfo.arrayLayers   = LayerCount;
    ImageInfo.format        = Format;
    ImageInfo.tiling        = VK_IMAGE_TILING_OPTIMAL;
    ImageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    ImageInfo.usage         = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | Usage;
    ImageInfo.samples       = VK_SAMPLE_COUNT_1_BIT;
    ImageInfo.sharingMode   = VK_SHARING_MODE_EXCLUSIVE;

    VmaAllocationCreateInfo AllocInfo {};
    AllocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;

    VkResult Result = vmaCreateImage(Context->Allocator, &ImageInfo, &AllocInfo, &OutImage, &OutAllocation, nullptr);
    
    if(Result != VK_SUCCESS)
    {
        return false;
    }

    VkDeviceSize SingleLayerSize = (VkDeviceSize)PixelByteSize* Width * Height;
    VkDeviceSize TotalSize = SingleLayerSize * LayerCount;

    VkBuffer StagingBuffer = VK_NULL_HANDLE;
    VkDeviceMemory StagingBufferMemory = VK_NULL_HANDLE;
    bool StageBufferResult = CreateStagingBuffer(
        Context,
        TotalSize,
        StagingBuffer,
        StagingBufferMemory);

    if(!StageBufferResult)
    {
        return false;
    }

    void* MappedPtr = nullptr;
    vkMapMemory(Context->Device, StagingBufferMemory, 0, TotalSize, 0, &MappedPtr);
    for(size_t i = 0; i<LayerCount; ++i)
    {
        memcpy(((char*)MappedPtr) + i * SingleLayerSize, Data[i], SingleLayerSize);
    }
    vkUnmapMemory(Context->Device, StagingBufferMemory);

	VkCommandBuffer Cmd = RHI->CreateSingletTimeCommandBuffer();
	
	SetImageLayout(
		Cmd,
		OutImage,
		VK_IMAGE_LAYOUT_UNDEFINED,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		VK_IMAGE_ASPECT_COLOR_BIT,
		VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
		VK_PIPELINE_STAGE_TRANSFER_BIT,
		LayerCount,
		1);
	
	CopyBuffer2Image2D(Cmd, StagingBuffer, OutImage, Width, Height, VK_IMAGE_ASPECT_COLOR_BIT, LayerCount, 0);

	RHI->EndSingleTimeCommandBuffer(Cmd);

	RHI->EndStageBuffer(StagingBuffer, StagingBufferMemory);

	return true;
}

bool VKTool::CreateBuffer(VulkanRHIContext* Context, VkDeviceSize BufferSize, VkBufferUsageFlags BufferUsage,
                          VmaMemoryUsage MemoryUsage, VkBuffer& OutBuffer, VmaAllocation& OutAllocation)
{
	VkBufferCreateInfo BufferInfo {};
	BufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	BufferInfo.size = BufferSize;
	BufferInfo.usage = BufferUsage;
	BufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	

	VmaAllocationCreateInfo MemoryAllocationInfo {};
	MemoryAllocationInfo.usage = MemoryUsage;

	VkResult Result = vmaCreateBufferWithAlignment(
		Context->Allocator,
		&BufferInfo,
		&MemoryAllocationInfo,
		(VkDeviceSize)4 * 4,
		&OutBuffer, &OutAllocation,
		nullptr);
	return Result == VK_SUCCESS;
}

bool VKTool::CreateStagingBuffer(VulkanRHIContext* Context, VkDeviceSize Size, VkBuffer& OutStageBuffer,
                                 VkDeviceMemory& OutMemory)
{
    VkBufferCreateInfo StageBufferInfo {};
    StageBufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    StageBufferInfo.size = Size;
    StageBufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    StageBufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VkResult Result = vkCreateBuffer(Context->Device, &StageBufferInfo, nullptr, &OutStageBuffer);
    if(Result != VK_SUCCESS)
    {
    	LOG_ERROR_FUNCTION("Create staging buffer memory fail");
        return false;
    }

    VkMemoryRequirements MemoryRequirements {};
    vkGetBufferMemoryRequirements(Context->Device, OutStageBuffer, &MemoryRequirements);
    static uint32_t StagingBufferIndex = FindMemoryIndex(
        Context,
        MemoryRequirements.memoryTypeBits,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    VkMemoryAllocateInfo MemoryAllocationInfo {};
    MemoryAllocationInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    MemoryAllocationInfo.allocationSize = MemoryRequirements.size;
    MemoryAllocationInfo.memoryTypeIndex = StagingBufferIndex;

    Result = vkAllocateMemory(Context->Device, &MemoryAllocationInfo, nullptr, &OutMemory);
    if(Result != VK_SUCCESS)
    {
    	LOG_ERROR_FUNCTION("Create staging buffer memory fail");
        return false;
    }

    Result = vkBindBufferMemory(Context->Device, OutStageBuffer, OutMemory, 0);
    return Result == VK_SUCCESS;
    
}

VkPipelineLayout VKTool::CreatePipelineLayout(VulkanRHIContext* Context,
	const std::vector<VkDescriptorSetLayout>& Layouts)
{
	VkPipelineLayoutCreateInfo LayoutInfo {};
	LayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;

	LayoutInfo.setLayoutCount = (uint32_t)Layouts.size();
	LayoutInfo.pSetLayouts = Layouts.data();

	VkPipelineLayout Layout = VK_NULL_HANDLE;
	VkResult Result = vkCreatePipelineLayout(Context->Device, &LayoutInfo, nullptr, &Layout);
	if(Result != VK_SUCCESS)
	{
		return VK_NULL_HANDLE;
	}

	return Layout;
}

VkPipelineShaderStageCreateInfo VKTool::CreateShaderStageInfo(VkShaderModule Module, VkShaderStageFlagBits ShaderStage)
{
	VkPipelineShaderStageCreateInfo ShaderStageInfo {};
	ShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	ShaderStageInfo.module = Module;
	ShaderStageInfo.stage = ShaderStage;
	ShaderStageInfo.pName = "main";

	return ShaderStageInfo;
}

VkPipelineRasterizationStateCreateInfo VKTool::CreateRasterizationInfo(VkPolygonMode PolyFillMode,
	VkCullModeFlags CullMode, VkFrontFace FrontFaceSpec, float LineWidth)
{
	VkPipelineRasterizationStateCreateInfo RasterizationInfo {};
	RasterizationInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	RasterizationInfo.depthClampEnable = VK_FALSE;
	RasterizationInfo.rasterizerDiscardEnable = VK_FALSE;
	RasterizationInfo.polygonMode = PolyFillMode;
	RasterizationInfo.lineWidth = LineWidth;
	RasterizationInfo.cullMode = CullMode;
	RasterizationInfo.frontFace = FrontFaceSpec;
	RasterizationInfo.depthBiasEnable = VK_FALSE;

	return RasterizationInfo;
}

VkPipelineColorBlendAttachmentState VKTool::CreateBlendAttachment(bool Enable)
{
	VkPipelineColorBlendAttachmentState ColorBlendAttachmentInfo {};
	ColorBlendAttachmentInfo.blendEnable = Enable ? VK_TRUE : VK_FALSE;
	ColorBlendAttachmentInfo.colorWriteMask = VK_COLOR_COMPONENT_R_BIT |
											  VK_COLOR_COMPONENT_G_BIT |
											  VK_COLOR_COMPONENT_B_BIT |
											  VK_COLOR_COMPONENT_A_BIT;
	return ColorBlendAttachmentInfo;
}

VkPipelineColorBlendStateCreateInfo VKTool::CreateColorBlendStageInfo(bool EnableLogic, VkLogicOp LogicOperation,
	const std::vector<VkPipelineColorBlendAttachmentState>& AttachmentBlend)
{
	VkPipelineColorBlendStateCreateInfo ColorBlendStateInfo {};
	ColorBlendStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	ColorBlendStateInfo.logicOpEnable = EnableLogic ? VK_TRUE : VK_FALSE;
	ColorBlendStateInfo.logicOp = LogicOperation;
	ColorBlendStateInfo.attachmentCount = (uint32_t)AttachmentBlend.size();
	ColorBlendStateInfo.pAttachments = AttachmentBlend.data();

	return ColorBlendStateInfo;
}

VkPipelineDepthStencilStateCreateInfo VKTool::CreateDepthStencilStageInfo(bool EnableDepth, bool EnableDepthWrite,
                                                                          bool EnableStencil, VkCompareOp DepthCompareOp)
{
	VkPipelineDepthStencilStateCreateInfo DepthStencilInfo {};
	DepthStencilInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	DepthStencilInfo.depthTestEnable = EnableDepth ? VK_TRUE : VK_FALSE;
	DepthStencilInfo.depthWriteEnable = EnableDepthWrite ? VK_TRUE : VK_FALSE;
	DepthStencilInfo.stencilTestEnable = EnableStencil ? VK_TRUE : VK_FALSE;
	DepthStencilInfo.depthCompareOp = DepthCompareOp;
	DepthStencilInfo.depthBoundsTestEnable = VK_FALSE;

	return DepthStencilInfo;
}

VkStencilOpState VKTool::CreateStencilOp(VkCompareOp StencilCompareOp, uint32_t StencilCompareMask,
	uint32_t StencilWriteMask, uint32_t StencilWriteValue, VkStencilOp OnPass, VkStencilOp OnFail,
	VkStencilOp OnDepthFail)
{
	VkStencilOpState StencilOpState {};
	StencilOpState.failOp = OnFail;
	StencilOpState.passOp = OnPass;
	StencilOpState.depthFailOp = OnDepthFail;
	StencilOpState.reference = StencilWriteValue;
	StencilOpState.compareMask = StencilCompareMask;
	StencilOpState.writeMask = StencilWriteMask;
	StencilOpState.compareOp = StencilCompareOp;

	return StencilOpState;
}

VkWriteDescriptorSet VKTool::CreateDescriptorWrite(VkDescriptorSet WriteSet, uint32_t Binding, VkDescriptorType Type)
{
	VkWriteDescriptorSet DescriptorSetWrite {};
	DescriptorSetWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	DescriptorSetWrite.dstSet = WriteSet;
	DescriptorSetWrite.dstBinding = Binding;
	DescriptorSetWrite.dstArrayElement = 0;
	DescriptorSetWrite.descriptorCount = 1;
	DescriptorSetWrite.descriptorType = Type;

	return DescriptorSetWrite;
}


void VKTool::LoadMipmapDataImage2D(VkCommandBuffer CmdBuffer, VulkanRHIContext* Context, VkImage ToImage, uint32_t Width,
                                   uint32_t Height, uint32_t PixelByteSize, VkImageAspectFlags AspectFlag, uint32_t LayerCount, uint32_t MipmapLevel, const std::vector<void*>& Data, uint32_t
                                   LayerBias, VkBuffer& OutStagingBuffer, VkDeviceMemory& OutStagingBufferMemory)
{
	OutStagingBuffer = VK_NULL_HANDLE;
	OutStagingBufferMemory = VK_NULL_HANDLE;
	if(MipmapLevel == 0)
	{
		LOG_ERROR_FUNCTION("First level mipmap(0) should loaded when creating texture");
		return;
	}
	VkDeviceSize SingleLayerSize =  (VkDeviceSize)PixelByteSize * Width * Height;
	VkDeviceSize TotalSize = SingleLayerSize * LayerCount;
	

	bool StageBufferResult = CreateStagingBuffer(
		Context,
		TotalSize,
		OutStagingBuffer,
		OutStagingBufferMemory);

	if(!StageBufferResult)
	{
		return;
	}

	void* MappedPtr = nullptr;
	vkMapMemory(Context->Device, OutStagingBufferMemory, 0, TotalSize, 0, &MappedPtr);
	for(uint32_t i = 0; i < LayerCount; ++i)
	{
		memcpy(((char*)MappedPtr + i * SingleLayerSize), Data[i], SingleLayerSize);	
	}
	vkUnmapMemory(Context->Device, OutStagingBufferMemory);
	
	CopyBuffer2Image2D(CmdBuffer, OutStagingBuffer, ToImage, Width, Height, AspectFlag, LayerCount, MipmapLevel, LayerBias);
}

VkSampler VKTool::CreateVulkanSampler(VulkanRHIContext* Context, VkSamplerAddressMode WrapU,
                                      VkSamplerAddressMode WrapV, VkSamplerAddressMode WrapW, VkFilter MinFilter, VkFilter MagFilter,
                                      VkSamplerMipmapMode MipmapFilter, uint32_t Anisotropy, uint32_t BorderColor)
{
	static float AnisotropyLimit = [Context]()
	{
		VkPhysicalDeviceProperties PhysicalDeviceInfo {};
		vkGetPhysicalDeviceProperties(Context->PhysicalDevice, &PhysicalDeviceInfo);
		return PhysicalDeviceInfo.limits.maxSamplerAnisotropy;
	}();

	VkSamplerCreateInfo SamplerInfo {};
	SamplerInfo.sType        = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	SamplerInfo.magFilter    = MagFilter;
	SamplerInfo.minFilter    = MinFilter;
	SamplerInfo.addressModeU = WrapU;
	SamplerInfo.addressModeV = WrapV;
	SamplerInfo.addressModeW = WrapW;

	float FAnisotropy = (float)Anisotropy;
	if(Anisotropy == 0)
	{
		SamplerInfo.anisotropyEnable = VK_FALSE;
	}
	else if(FAnisotropy < AnisotropyLimit)
	{
		SamplerInfo.anisotropyEnable = VK_TRUE;
		SamplerInfo.maxAnisotropy    = FAnisotropy;
	} else
	{
		SamplerInfo.anisotropyEnable = VK_TRUE;
		SamplerInfo.maxAnisotropy    = AnisotropyLimit;
	}
	
	if((BorderColor >> 8) > 0)
	{
		SamplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_WHITE;
	}
	else
	{
		SamplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
	}
	
	SamplerInfo.unnormalizedCoordinates = VK_FALSE;
	SamplerInfo.compareEnable           = VK_FALSE;
	SamplerInfo.compareOp               = VK_COMPARE_OP_ALWAYS;
	SamplerInfo.mipmapMode              = MipmapFilter;

	SamplerInfo.minLod = 0.0f;
	SamplerInfo.maxLod = std::numeric_limits<float>::max();

	VkSampler Sampler = VK_NULL_HANDLE;
	VkResult Result = vkCreateSampler(Context->Device, &SamplerInfo, nullptr, &Sampler);

	if(Result == VK_SUCCESS)
	{
		return Sampler;
	}

	return VK_NULL_HANDLE;
}

void VKTool::GenerateMipmapImage2D(VkCommandBuffer CmdBuffer, VulkanRHIContext* Context, VkImage ToImage,
                                   uint32_t OriWidth, uint32_t OriHeight, VkImageAspectFlags AspectFlag, uint32_t LayerCount, uint32_t MipmapLevel,
                                   VkFormat InternalFormat, uint32_t LayerBias)
{
	if(OriWidth <= 1 && OriHeight <= 1)
	{
		LOG_ERROR_FUNCTION(
			"Cannot generate mipmap level {0}, image size({1}, {2}) too low",
			MipmapLevel,
			OriWidth,
			OriHeight);
		return;
	}

	if(MipmapLevel == 0)
	{
		LOG_ERROR_FUNCTION("Cannot generate mipmap at level 0");
		return;
	}

	bool FormatSupport = false;
	static std::unordered_map<VkFormat, bool> BlitTable;
	auto Iter = BlitTable.find(InternalFormat);
	if(Iter == BlitTable.end())
	{
		VkFormatProperties FormatProperties {};
		vkGetPhysicalDeviceFormatProperties(Context->PhysicalDevice, InternalFormat, &FormatProperties);
		FormatSupport = FormatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT;
		BlitTable.emplace(InternalFormat, FormatSupport);
	}
	else
	{
		FormatSupport = Iter->second;
	}

	if(!FormatSupport)
	{
		LOG_ERROR_FUNCTION("Cannot generate mipmap, image format not support");
		return;
	}

	VkImageBlit BlitInfo {};
	BlitInfo.srcOffsets[0]                 = {0, 0, 0};
	BlitInfo.srcOffsets[1]                 = {(int32_t)OriWidth, (int32_t)OriHeight, 1};
	BlitInfo.srcSubresource.aspectMask     = AspectFlag;
	BlitInfo.srcSubresource.mipLevel       = MipmapLevel - 1;
	BlitInfo.srcSubresource.baseArrayLayer = LayerBias;
	BlitInfo.srcSubresource.layerCount     = LayerCount; 

	BlitInfo.dstOffsets[0]                 = {0, 0, 0};
	BlitInfo.dstOffsets[1]                 = {(int32_t)(OriWidth >> 1), (int32_t)(OriHeight >> 1), 1};
	BlitInfo.dstSubresource.aspectMask     = AspectFlag;
	BlitInfo.dstSubresource.mipLevel       = MipmapLevel;
	BlitInfo.dstSubresource.baseArrayLayer = LayerBias;
	BlitInfo.dstSubresource.layerCount     = LayerCount;

	vkCmdBlitImage(
		CmdBuffer,
		ToImage,
		VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
		ToImage,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		1,
		&BlitInfo,
		VK_FILTER_LINEAR);
}


void VKTool::SetImageLayout(VkCommandBuffer CmdBuffer, VkImage Image, VkImageLayout OldImageLayout,
                            VkImageLayout NewImageLayout, VkImageAspectFlags AspectFlag, VkPipelineStageFlags SrcStageMask,
                            VkPipelineStageFlags DstStageMask, uint32_t LayerCount, uint32_t MipmapCount, uint32_t LayerBias, uint32_t MipmapBias)
{
    // Create an image barrier object
	VkImageMemoryBarrier ImageMemoryBarrier {};
	ImageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	ImageMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	ImageMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	ImageMemoryBarrier.oldLayout = OldImageLayout;
	ImageMemoryBarrier.newLayout = NewImageLayout;
	ImageMemoryBarrier.image = Image;
	ImageMemoryBarrier.subresourceRange.aspectMask = AspectFlag;
	ImageMemoryBarrier.subresourceRange.layerCount = LayerCount;
	ImageMemoryBarrier.subresourceRange.baseArrayLayer = LayerBias;
	ImageMemoryBarrier.subresourceRange.levelCount = MipmapCount;
	ImageMemoryBarrier.subresourceRange.baseMipLevel = MipmapBias;
	
	
	// Source layouts (old)
	// Source access mask controls actions that have to be finished on the old layout
	// before it will be transitioned to the new layout
	switch (OldImageLayout)
	{
	case VK_IMAGE_LAYOUT_UNDEFINED:
		// Image layout is undefined (or does not matter)
		// Only valid as initial layout
		// No flags required, listed only for completeness
		ImageMemoryBarrier.srcAccessMask = 0;
		break;
	case VK_IMAGE_LAYOUT_PREINITIALIZED:
		// Image is preinitialized
		// Only valid as initial layout for linear images, preserves memory contents
		// Make sure host writes have been finished
		ImageMemoryBarrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT;
		break;
	case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
		// Image is a color attachment
		// Make sure any writes to the color buffer have been finished
		ImageMemoryBarrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		break;
	case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
		// Image is a depth/stencil attachment
		// Make sure any writes to the depth/stencil buffer have been finished
		ImageMemoryBarrier.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
		break;
	case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
		// Image is a transfer source
		// Make sure any reads from the image have been finished
		ImageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
		break;
	case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
		// Image is a transfer destination
		// Make sure any writes to the image have been finished
		ImageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		break;
	case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
		// Image is read by a shader
		// Make sure any shader reads from the image have been finished
		ImageMemoryBarrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
		break;
	default:
		// Other source layouts aren't handled (yet)
		break;
	}
	// Target layouts (new)
	// Destination access mask controls the dependency for the new image layout
	switch (NewImageLayout)
	{
	case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
		// Image will be used as a transfer destination
		// Make sure any writes to the image have been finished
		ImageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		break;
	case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
		// Image will be used as a transfer source
		// Make sure any reads from the image have been finished
		ImageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
		break;
	case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
		// Image will be used as a color attachment
		// Make sure any writes to the color buffer have been finished
		ImageMemoryBarrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		break;
	case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
		// Image layout will be used as a depth/stencil attachment
		// Make sure any writes to depth/stencil buffer have been finished
		ImageMemoryBarrier.dstAccessMask = ImageMemoryBarrier.dstAccessMask | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
		break;
	case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
		// Image will be read in a shader (sampler, input attachment)
		// Make sure any writes to the image have been finished
		if (ImageMemoryBarrier.srcAccessMask == 0)
		{
			ImageMemoryBarrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT | VK_ACCESS_TRANSFER_WRITE_BIT;
		}
		ImageMemoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
		break;
	default:
		// Other source layouts aren't handled (yet)
		break;
	}
	// Put barrier inside setup command buffer
	vkCmdPipelineBarrier(
		CmdBuffer,
		SrcStageMask,
		DstStageMask,
		0,
		0,
		nullptr,
		0,
		nullptr,
		1, &ImageMemoryBarrier);
}

void VKTool::CopyBuffer2Image2D(VkCommandBuffer CmdBuffer, VkBuffer Buffer, VkImage Image, uint32_t Width,
                                uint32_t Height, VkImageAspectFlags AspectFlags, uint32_t LayerCount, uint32_t MipmapLevel, uint32_t LayerBias,
                                uint32_t BufferOffset, int32_t ImageOffsetU, int32_t ImageOffsetV)
{
	VkBufferImageCopy Region {};
	Region.bufferOffset                    = BufferOffset;
	Region.bufferRowLength                 = 0;
	Region.bufferImageHeight               = 0;
	Region.imageSubresource.aspectMask     = AspectFlags;
	Region.imageSubresource.mipLevel       = MipmapLevel;
	Region.imageSubresource.baseArrayLayer = LayerBias;
	Region.imageSubresource.layerCount     = LayerCount;
	Region.imageOffset                     = {ImageOffsetU, ImageOffsetV, 0};
	Region.imageExtent                     = {Width, Height, 1};

	vkCmdCopyBufferToImage(CmdBuffer, Buffer, Image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &Region);
}

void VKTool::CopyBuffer2Buffer(VkCommandBuffer CmdBuffer, VkBuffer SrcBuffer, VkBuffer DstBuffer,
                               VkDeviceSize Size, VkDeviceSize SrcOffset, VkDeviceSize DstOffset)
{
	VkBufferCopy BufferCopy {};
	BufferCopy.srcOffset = SrcOffset;
	BufferCopy.dstOffset = DstOffset;
	BufferCopy.size = Size;

	vkCmdCopyBuffer(CmdBuffer, SrcBuffer, DstBuffer, 1, &BufferCopy);
}

void VKTool::CopyImage2Image2D(VkCommandBuffer CmdBuffer, VkImage Src, VkImage Dst, uint32_t Width,
                               uint32_t Height, VkImageAspectFlags Aspect, uint32_t LayerCount, int32_t SrcImageOffsetU,
                               int32_t SrcImageOffsetV, int32_t DstImageOffsetU, int32_t DstImageOffsetV, uint32_t SrcMipmapLevel, uint32_t DstMipmapLevel, uint32_t SrcLayerBias, uint32_t DstLayerBias)
{
	VkImageCopy ImageCopy {};
	ImageCopy.extent = {Width, Height, 1};
	ImageCopy.srcOffset = {SrcImageOffsetU, SrcImageOffsetV, 0};
	ImageCopy.srcSubresource.aspectMask = Aspect;
	ImageCopy.srcSubresource.layerCount = LayerCount;
	ImageCopy.srcSubresource.mipLevel = SrcMipmapLevel;
	ImageCopy.srcSubresource.baseArrayLayer = SrcLayerBias;

	ImageCopy.dstOffset = {DstImageOffsetU, DstImageOffsetV, 0};
	ImageCopy.dstSubresource.aspectMask = Aspect;
	ImageCopy.dstSubresource.layerCount = LayerCount;
	ImageCopy.dstSubresource.mipLevel = DstMipmapLevel;
	ImageCopy.dstSubresource.baseArrayLayer = DstLayerBias;

	vkCmdCopyImage(
		CmdBuffer,
		Src,
		VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
		Dst,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		1,
		&ImageCopy);
}


