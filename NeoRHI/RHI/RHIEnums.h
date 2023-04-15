#pragma once
#include <cstdint>

#define RHI_BITMAST_ENUM_CLASS(CassName) \
    inline CassName operator | (CassName A, CassName B) \
    {\
        return CassName(uint32_t(A) | uint32_t(B));\
    } \
    inline CassName operator & (CassName A, CassName B)\
    {\
        return CassName(uint32_t(A) & uint32_t(B)); \
    }\
    inline CassName operator ~ (CassName A)\
    {\
        return CassName(~uint32_t(A));\
    } \
    inline bool operator !(CassName A)\
    {\
        return uint32_t(A) == 0;\
    } \
    inline bool operator ==(CassName A, uint32_t B)\
    {\
        return uint32_t(A) == B;\
    }\
    inline bool operator !=(CassName A, uint32_t B)\
    {\
        return uint32_t(A) != B;\
    }

/*
 * Universal: format and resource state
 */
enum class RHIFormat : uint16_t
{
    UNDEFINED = 0,

    R8_UINT,
    R8_SINT,
    R8_UNORM,
    R8_SNORM,
    RG8_UINT,
    RG8_SINT,
    RG8_UNORM,
    RG8_SNORM,
    R16_UINT,
    R16_SINT,
    R16_UNORM,
    R16_SNORM,
    R16_FLOAT,
    BGRA4_UNORM,
    B5G6R5_UNORM,
    BGR5A1_UNORM,
    RGBA8_UINT,
    RGBA8_SINT,
    RGBA8_UNORM,
    RGBA8_SNORM,
    BGRA8_UNORM,
    RGBA8_SRGB,
    BGRA8_SRGB,
    A2_RGB10_HDR,
    RG11B10_FLOAT,
    RG16_UINT,
    RG16_SINT,
    RG16_UNORM,
    RG16_SNORM,
    RG16_FLOAT,
    R32_UINT,
    R32_SINT,
    R32_FLOAT,
    RGBA16_UINT,
    RGBA16_SINT,
    RGBA16_FLOAT,
    RGBA16_UNORM,
    RGBA16_SNORM,
    RG32_UINT,
    RG32_SINT,
    RG32_FLOAT,
    RGB32_UINT,
    RGB32_SINT,
    RGB32_FLOAT,
    RGBA32_UINT,
    RGBA32_SINT,
    RGBA32_FLOAT,
        
    D16,
    D24S8,
    D32,
    D32S8,

    BC1_UNORM,
    BC1_UNORM_SRGB,
    BC2_UNORM,
    BC2_UNORM_SRGB,
    BC3_UNORM,
    BC3_UNORM_SRGB,
    BC4_UNORM,
    BC4_SNORM,
    BC5_UNORM,
    BC5_SNORM,
    BC6H_UFLOAT,
    BC6H_SFLOAT,
    BC7_UNORM,
    BC7_SRGB,

    MAX
};

enum class RHIResourceState : uint32_t
{
    Undefined      = 0,
    
    General        = (1),
    
    ShaderRead     = (1 << 1),
    ShaderWrite    = (1 << 2),
    RenderTarget   = (1 << 3), 
    DepthRead      = (1 << 4),
    DepthWrite     = (1 << 5),
    Present        = (1 << 6) ,
    TransferSrc    = (1 << 7),
    TransferDst    = (1 << 8),
    ResolveSrc     = (1 << 9),
    ResolveDst     = (1 << 10),
    VertexBuffer   = (1 << 11),
    IndexBuffer    = (1 << 12),
    IndirectBuffer = (1 << 13),
    UniformBuffer  = (1 << 14),
};

RHI_BITMAST_ENUM_CLASS(RHIResourceState)

/*
 * Texture
 */

enum class TextureType
{
    Texture1D = 0,
    Texture1DArray,
    Texture2D,
    Texture2DArray,
    Texture2DCube,
    Texture2DCubeArray,

    MAX //No texture3d support currently
};

enum class TextureUsage : uint32_t
{
    None                = 0,
    RenderTarget        = 1,
    DepthStencilTexture = (1 << 1),
    UAVTexture          = (1 << 2),
    SRVTexture          = (1 << 3), 
};

RHI_BITMAST_ENUM_CLASS(TextureUsage)

enum class TexturePlaneBit : uint32_t
{
    None    = 0,
    Color   = 1,
    Depth   = (1 << 1),
    Stencil = (1 << 2),
};

RHI_BITMAST_ENUM_CLASS(TexturePlaneBit)

/*
 * Buffer
 */

enum class BufferUsage : uint32_t
{
    None           = 0,
    UniformBuffer  = 1,
    UAVBuffer      = (1 << 1),
    SRVBuffer      = (1 << 2),
    VertexBuffer   = (1 << 3),
    IndexBuffer    = (1 << 4),
    IndirectBuffer = (1 << 5)
};

RHI_BITMAST_ENUM_CLASS(BufferUsage)

enum class MemoryFlag : uint32_t
{
    None         = 0,
    GPULocal     = 1,
    HostVisible  = (1 << 1),
    HostCoherent = (1 << 2),
    HostCache    = (1 << 3)
};

/*
 * Sampler
 */

enum class SamplerType
{
    Nearest = 0,
    Linear,
    Cubic,

    MAX
};

enum class SampleWrap
{
    Repeat = 0,
    MirrorRepeat,
    Clamp2Edge,
    Clamp2Border,

    MAX
};

enum class SamplerMipmapFilter
{
    Nearest = 0,
    Linear,

    MAX
};

/*
 * Shader stage
 */

enum class ShaderStage : uint32_t
{
    None     = 0,
    Task     = 1,
    Mesh     = (1 << 1),
    Vertex   = (1 << 2),
    Fragment = (1 << 3),

    Compute  = (1 << 4),
};

RHI_BITMAST_ENUM_CLASS(ShaderStage)

/*
 * Descriptor
 */
enum class DescriptorType
{
    Sampler = 0,
    TextureSRV,
    TextureUAV,
    ByteBufferSRV,
    ByteBufferUAV,
    TypedBufferSRV,
    TypedBufferUAV,
    StructuredBufferSRV,
    StructuredBufferUAV,
    UniformBufferCBV,
    DynamicUniformBufferCBV,

    MAX
};

/*
 * Task type
 */

enum class RenderTaskType
{
    Graphics = 0,
    Compute,

    MAX
};

/*
 * Pipeline
 */

enum class CompareFunction
{
    Never = 0,
    Less,
    Equal,
    LEqual,
    Greater,
    NEqual,
    GEqual,
    Always,

    MAX
};

enum class StencilOperation
{
    Keep = 0,
    Zero,
    Replace,
    IncreaseAndClamp,
    DecreaseAndClamp,
    Invert,
    IncreaseAndWrap,
    DecreaseAndWrap,

    MAX
};

enum class FillMode
{
    Fill = 0,
    Line,
    Point,

    MAX
};

enum class CullMode
{
    None = 0,
    Front,
    Back,

    MAX
};

enum class FrontFace
{
    Clockwise = 0,
    CounterClockwise,

    MAX
};

enum class BlendFactor
{
    Zero = 0,
    One,
    SrcColor,
    OneMinusSrcColor,
    DstColor,
    OneMinusDstColor,
    ConstantColor,
    OneMinusConstantColor,
    SrcAlpha,
    OneMinusSrcAlpha,
    DstAlpha,
    OneMinusDstAlpha,
    ConstantAlpha,
    OneMinusConstantAlpha,

    MAX
};

enum class BlendOperation
{
    Add = 0,
    Subtract,
    ReverseSubtract,
    Min,
    Max,

    MAX
};

enum class BlendLogicOp
{
    Clear = 0,
    And,
    AndReverse = 2,
    Copy = 3,
    AndInverted = 4,
    Not = 5,
    XOr = 6,
    Or = 7,
    NOr = 8,
    Equivalent = 9,
    Invert = 10,
    OrReverse = 11,
    CopyInverted = 12,
    OrInverted = 13,
    NAnd = 14,
    Set = 15,
};

enum class ColorMask : uint8_t
{
    None = 0,
    R    = 1,
    G    = (1 << 1),
    B    = (1 << 2),
    A    = (1 << 3),

    AllColor = 7,
    All      = 15
};

RHI_BITMAST_ENUM_CLASS(ColorMask)

enum class PipelineStage : uint32_t
{
    None         = 0,
    Start        = 1,
    Indirect     = 1 << 1,
    Vertex       = 1 << 2,
    EarlyDepth   = 1 << 3,
    Fragment     = 1 << 4,
    Late_Depth   = 1 << 5,
    Task         = 1 << 6,
    Mesh         = 1 << 7,
    Compute      = 1 << 8,
    OutputColor  = 1 << 9,
    Transfer     = 1 << 10,
    End          = 1 << 11,
    Host         = 1 << 12,

    All         = 0xFFF,
    AllGraphics = 0x3F | OutputColor | End,
};

RHI_BITMAST_ENUM_CLASS(PipelineStage)