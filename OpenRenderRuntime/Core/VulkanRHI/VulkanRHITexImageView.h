#pragma once
#include "VulkanRHITexImage.h"
#include "OpenRenderRuntime/Core/RHI/RHITexImageView.h"

class VulkanRHITexImageView : public RHITexImageView
{
	
public:
	VkImageView ImageView = VK_NULL_HANDLE;
};
