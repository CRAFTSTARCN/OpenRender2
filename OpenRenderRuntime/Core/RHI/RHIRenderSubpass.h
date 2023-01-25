#pragma once

#include "OpenRenderRuntime/Core/RHI/RHITexture.h"

#include "OpenRenderRuntime/Core/RHI/RHIEnums.h"

#define SUBPASS_DEPEND_PASS_EXTERNAL (~(0u))

struct SubpassAttachmentRef
{
	uint32_t Index = 0;
	TexImageLayout RequireLayout = TexImageLayout_Color_Attachment;
};

struct SubpassDependency
{
	uint32_t DependPass = SUBPASS_DEPEND_PASS_EXTERNAL;
	PipelineStage SrcStage = PipelineStageBit_End;
	PipelineStage DstStage = PipelineStageBit_Start;
	AccessMask SrcMask = AccessMaskBit_Shader_Write | AccessMaskBit_Color_Attachment_Write;
	AccessMask DstMask = AccessMaskBit_Shader_Read | AccessMaskBit_Color_Attachment_Read;
	bool InRegion = true;
};

struct SubpassInfo
{
	std::vector<SubpassAttachmentRef> InputAttachment {};
	std::vector<SubpassAttachmentRef> ColorAttachment {};
	SubpassAttachmentRef DepthStencilAttachment {};
	std::vector<SubpassDependency> Dependency;
	std::vector<SubpassDependency> ExternalDependency;
	bool RequireDepthStencil = false;
};

class RHIRenderSubpass
{
	
public:
	
};
