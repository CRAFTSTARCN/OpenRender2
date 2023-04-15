#pragma once
#include <vector>

#include "RHIDescriptor.h"
#include "RHIEnums.h"
#include "RHIShader.h"

struct StencilOperationDesc
{
    CompareFunction StencilFunction = CompareFunction::Never;
    StencilOperation OnPass = StencilOperation::Keep;
    StencilOperation OnFail = StencilOperation::Keep;
    StencilOperation OnDepthFail = StencilOperation::Keep;
};

struct DepthStencilDesc
{
    bool EnableDepthTest = false;
    bool EnableDepthWrite = false;
    CompareFunction DepthFunction = CompareFunction::Never;

    bool EnableStencilTest = false;
    uint32_t StencilReadMask = 0xFF;
    uint32_t StencilWriteMask = 0xFF;
    uint32_t StencilValue = 0;
    StencilOperation FrontFaceOp {};
    StencilOperation BackFaceOp {};
};

struct RasterizationDesc
{
    FrontFace FrontFaceWise = FrontFace::Clockwise;
    CullMode CullFace = CullMode::None;
    FillMode Fill = FillMode::Fill;
    uint32_t SampleBit = 1;
};

struct RenderTargetColorBlendingDesc
{
    BlendFactor SrcColorFactor = BlendFactor::Zero;
    BlendFactor DstColorFactor = BlendFactor::Zero;
    BlendOperation ColorOperation = BlendOperation::Add;

    BlendFactor SrcAlphaFactor = BlendFactor::Zero;
    BlendFactor DstAlphaFactor = BlendFactor::Zero;
    BlendOperation AlphaOperation = BlendOperation::Add;

    ColorMask WriteMask = ColorMask::None;
    bool EnableColorBlending = false;
};

struct ColorBlendDesc
{
    RenderTargetColorBlendingDesc RTBlending[8] = {};
    float ConstantColor[4] = {};
    bool EnableLogicBlending = false;
    BlendLogicOp LogicOperation = BlendLogicOp::And;
    
};

struct FrameBufferInfo
{
    RHIFormat RenderTargetFormats[8] = {RHIFormat::UNDEFINED};
    RHIFormat DepthStencilFormat = RHIFormat::UNDEFINED;
};

struct GraphicsPSODesc
{
    std::vector<RHIDescriptorLayout*> Layouts;
    RHIShader* TaskShader = nullptr;
    RHIShader* MeshShader = nullptr;
    RHIShader* FragmentShader = nullptr;

    RasterizationDesc RasterStage {};
    DepthStencilDesc DepthStencilStage {};
    ColorBlendDesc ColorBlending {};
    FrameBufferInfo PSOFrameBufferInfo {};

    uint32_t RTCount = false;
    bool HasDepthStencil = false;
    
};

class RHIGraphicsPSO
{
    GraphicsPSODesc Desc;

public:

    virtual ~RHIGraphicsPSO() = default;

    const GraphicsPSODesc& GetDesc() {return Desc;}
    
};

