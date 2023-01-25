#pragma once
#include "OpenRenderRuntime/Core/RHI/RenderWindowProxy.h"
#include "OpenRenderRuntime/Core/RHI/RHIRenderPass.h"

class VulkanRHIRenderPass : public RHIRenderPass
{
	
public:
	VkRenderPass RenderPass = VK_NULL_HANDLE;
};