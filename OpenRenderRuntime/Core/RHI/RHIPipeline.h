#pragma once

#include "OpenRenderRuntime/Core/RHI/RHIEnums.h"

struct ColorBlendingInfo
{
	BlenderFactor SrcColorFactor = BlenderFactor_Src_Alpha;
	BlenderFactor DstColorFactor = BlenderFactor_One_Minus_Src_Alpha;
	BlendOperation ColorOperation = BlendOperation_Add;

	BlenderFactor SrcAlphaFactor = BlenderFactor_Src_Alpha;
	BlenderFactor DstAlphaFactor = BlenderFactor_One_Minus_Src_Alpha;
	BlendOperation AlphaOperation = BlendOperation_Add;
};

struct GraphicsPipelineFixedFunctionInfo
{
	//Depth
	bool EnableDepth = true;
	bool EnableDepthWrite = true;
	CompareOperation DepthCompareOperation = CompareOperation_LEqual;

	//Stencil
	bool EnableStencil = false;
	CompareOperation StencilCompareOperation = CompareOperation_Equal;
	uint32_t StencilCompareMask = ~(0u);
	uint32_t StencilWriteMask = ~(0u);
	uint32_t StencilWriteValue = 0;
	StencilTestOperation OnPass = StencilTestOperation_Replace;
	StencilTestOperation OnFail = StencilTestOperation_Keep;
	StencilTestOperation OnDepthFail = StencilTestOperation_Keep;

	//Pipeline rasterization
	FillMode RasterizationFill = FillMode_Fill;
	CullFace RasterizationCullFace = CullFace_None;
	FrontFace Front = FrontFace_Counter_Clockwise;
	float LineWidth = 1.0f;

	//Color blending
	bool EnableBlending = false;
	std::vector<ColorBlendingInfo> ColorBlendingInfos {};
};

class RHIPipeline
{
    
public:

};