#pragma once
#include <vector>

#include "OpenRenderRuntime/Core/RHI/RHIRenderSubpass.h"

#include "OpenRenderRuntime/Core/RHI/RHIEnums.h"


struct AttachmentDescription
{
	TexturePixelFormat Format = TexturePixelFormat_RGB8;
	AttachmentLoadOperation Onload = AttachmentLoadOperation_Nothing;
	AttachmentStoreOperation OnStore = AttachmentStoreOperation_Nothing;

	AttachmentLoadOperation OnStencilLoad = AttachmentLoadOperation_Nothing;
	AttachmentStoreOperation OnStencilStore = AttachmentStoreOperation_Nothing;

	TexImageLayout InitialLayout = TexImageLayout_Undefined;
	TexImageLayout FinalLayout = TexImageLayout_Undefined;
};

struct ClearColorInfo
{
	float Color[4] = {0.0f, 0.0f, 0.0f, 0.0f};
	float Depth = 0.0f;
	uint32_t Stencil = 0;
	bool IsDepthStencil = false;
};


class RHIRenderPass
{
	
public:
	std::vector<RHIRenderSubpass*> Passes {};
};
