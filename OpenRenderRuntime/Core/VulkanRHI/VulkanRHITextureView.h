#pragma once
#include "OpenRenderRuntime/Core/RHI/RHITextureView.h"

class VulkanRHITextureView : public RHITextureView
{
	
public:
	
	VkImageView ImageView = VK_NULL_HANDLE;
};
