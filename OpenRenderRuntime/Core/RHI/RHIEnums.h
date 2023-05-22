#pragma once
#include <cstdint>
/*
 * Buffer type
 */

enum BufferUsageBit
{
	BufferUsageBit_Vertex   = 1,
	BufferUsageBit_Index    = 1 << 1,
	BufferUsageBit_Storage  = 1 << 2,
	BufferUsageBit_Uniform  = 1 << 3,
	BufferUsageBit_Indirect = 1 << 4
};
typedef uint32_t BufferUsage;

/*
 * Buffer memory usage
 */ 
enum BufferMemoryUsage
{
	BufferMemoryUsage_GPU_Only = 1,
	BufferMemoryUsage_Host_Coherent = 2,
	BufferMemoryUsage_CPU_2_GPU = 3,
	BufferMemoryUsage_CPU_Copy= 4
};

/*
 * Material param type
 */ 
enum MaterialParamType
{
	MaterialParamType_Vector4Param = 0,
	MaterialParamType_ScalarParam = 1,
	MaterialParamType_Texture = 2,
	MaterialParamType_Texture_Cube = 3
};

/*
 * Pipeline color blend mode
 */
enum PipelineBlendMode
{
	PipelineBlendMode_Opaque,
	PipelineBlendMode_Masked,

	//Not implement yet
	PipelineBlendMode_Translucent,
	PipelineBlendMode_Additive
};


/*
 * Texture usage
 */
enum TextureUsageBit
{
	TextureUsageBit_Sample                   = 1,
	TextureUsageBit_Storage_Image            = 1 << 1,
	TextureUsageBit_Transfer_Dst             = 1 << 2,
	TextureUsageBit_Transfer_Src             = 1 << 3,
	TextureUsageBit_InputAttachment          = 1 << 4,
	TextureUsageBit_ColorAttachment          = 1 << 5,
	TextureUsageBit_DepthStencil             = 1 << 6,
	TextureUsageBit_TransientInputAttachment = 1 << 7,
};

typedef uint32_t TextureUsage;

enum TexturePlaneBit
{
	TexturePlaneBit_Color   = 0,
	TexturePlaneBit_Depth   = 1,
	TexturePlaneBit_Stencil = 1 << 1
};

typedef uint32_t TexturePlane;


/*
 * Render queue
 */
enum RenderingTaskQueue
{
	RenderingTaskQueue_Graphics = 0,
	RenderingTaskQueue_Compute = 1
};

/*
 * Attachment load operation
 * Direct vulkan cast
 */
enum AttachmentLoadOperation
{
	AttachmentLoadOperation_Load = 0,
	AttachmentLoadOperation_Clear = 1,
	AttachmentLoadOperation_Nothing = 2,
};

/*
 * Attachment store operation
 * Direct vulkan cast
 */
enum AttachmentStoreOperation
{
	AttachmentStoreOperation_Store = 0,
	AttachmentStoreOperation_Nothing = 1,
};

/*
 * 'GPU' pipeline stages
 */
enum PipelineStageBit
{
	PipelineStageBit_Start        = 1,
	PipelineStageBit_Vertex       = 1 << 1,
	PipelineStageBit_Early_Depth  = 1 << 2,
	PipelineStageBit_Fragment     = 1 << 3,
	PipelineStageBit_Late_Depth   = 1 << 4,
	PipelineStageBit_Task         = 1 << 5,
	PipelineStageBit_Mesh         = 1 << 6,
	PipelineStageBit_Compute      = 1 << 7,
	PipelineStageBit_Output_Color = 1 << 8,
	PipelineStageBit_Transfer     = 1 << 9,
	PipelineStageBit_End          = 1 << 10,
	PipelineStageBit_Host         = 1 << 11,

	PipelineStageBit_All_Graphics = 1 << 12,
	PipelineStageBit_All_Command  = 1 << 13,
	PipelineStageBit_Indirect     = 1 << 14
};
typedef uint32_t PipelineStage;

/*
 * Image layout, image is for usage
 */
enum TextureStatus
{
	TextureStatus_Undefined = 0,
	TextureStatus_General = 1,
	TextureStatus_Color_Attachment = 2,
	TextureStatus_Depth_Stencil_Attachment = 3,
	TextureStatus_Depth_Stencil_Readonly = 4,
	TextureStatus_Shader_Read = 5,
	TextureStatus_Transfer_Src = 6,
	TextureStatus_Transfer_Dst = 7,

	TextureStatus_Shader_Write = 10,
	TextureStatus_Resolve_Src  = 11,
	TextureStatus_Resolve_Dst  = 12,

	TextureStatus_Depth_Attachment = 1000241000,
	TextureStatus_Depth_ReadOnly = 1000241001,
	TextureStatus_Stencil_Attachment = 1000241002,
	TextureStatus_Stencil_ReadOnly = 1000241003,
	TextureStatus_For_Present = 1000001002
};

/*
 * Memory access mask
 * Direct vulkan cast
 */
enum AccessMaskBit
{
	AccessMaskBit_Indirect_Command_Read = 1,
	AccessMaskBit_Index_Read = 1 << 1,
	AccessMaskBit_Vertex_Attribute_Read = 1 << 2,
	AccessMaskBit_Uniform_Read = 1 << 3,
	AccessMaskBit_Input_Attachment_Read = 1 << 4,
	AccessMaskBit_Shader_Read = 1 << 5,
	AccessMaskBit_Shader_Write = 1 << 6,
	AccessMaskBit_Color_Attachment_Read = 1 << 7,
	AccessMaskBit_Color_Attachment_Write = 1 << 8,
	AccessMaskBit_Depth_Stencil_Read = 1 << 9,
	AccessMaskBit_Depth_Stencil_Write = 1 << 10,
	AccessMaskBit_Host_Read = 1 << 11,
	AccessMaskBit_Host_Write = 1 << 12
};
typedef uint32_t AccessMask;

/*
 * Pixel format
 */
enum RHIFormat
{
	RHIFormat_None = 0,
	RHIFormat_RGB8 = 1,
	RHIFormat_RGB8_SRGB = 2,
	RHIFormat_RGB32_FLOAT = 3,
    
	RHIFormat_RGBA8 = 4,
	RHIFormat_RGBA8_SRGB = 5,
	RHIFormat_RGBA32_FLOAT = 6,
    
	RHIFormat_R32_FLOAT = 7,
	RHIFormat_D32_FLOAT = 8,

	RHIFormat_BGRA8 = 9,
	RHIFormat_BGRA8_SRGB = 10,
	RHIFormat_RGBA16_FLOAT = 11,
	RHIFormat_A2_BGR10_HDR = 12,

	RHIFormat_D32_FLOAT_S8_UINT = 13,
	RHIFormat_S8_UINT = 14,
};

/*
 * Texture type (2D, Cube, Array)
 * TODO: Support texture cube array
 */
enum TextureType
{
	TextureType_2D,
	TextureType_Cube,
	TextureType_Array
};

/*
 * Wrap
 */
enum TextureWrap
{
	TextureWrap_Repeat = 0,
	TextureWrap_MirrorRepeat = 1,
	TextureWrap_Clamp2Edge = 2,
	TextureWrap_Clamp2Border = 3
};

/*
 * Filter
 */
enum FilterType
{
	FilterType_Nearest = 0,
	FilterType_Linear = 1,
	FilterType_Cubic = 2
};

/*
 * Mipmap sampler filter
 */
enum MipmapFilterType
{
	MipmapFilterType_Nearest = 0,
	MipmapFilterType_Linear = 1
};

/*
 * Texture usage at geometry pipeline or shading
 */
enum ParamUsageBit
{
	ParamUsageBit_Fragment = 1,
	ParamUsageBit_Geometry = 1 << 1
};
typedef uint32_t ParamUsage;

/*
 * A shader stage, for descriptor usage and shader module creation
 * Vulkan direct cast
 */
enum ShaderStageTypeBit
{
	ShaderStageTypeBit_Vertex = 1,
	ShaderStageTypeBit_Fragment = 1 << 4,
	ShaderStageTypeBit_Compute = 1 << 5,
	ShaderStageTypeBit_Task = 1 << 6,
	ShaderStageTypeBit_Mesh = 1 << 7
};
typedef uint32_t ShaderStageType;

/*
 * Descriptor type
 */
enum DescriptorType
{
	DescriptorType_Texture_With_Sampler = 0, 
	DescriptorType_Storage_Image = 1,
	DescriptorType_Uniform_Buffer = 2,
	DescriptorType_Storage_Buffer = 3,
	DescriptorType_Uniform_Buffer_Dynamic = 4,
	DescriptorType_Storage_Buffer_Dynamic = 5,
	DescriptorType_Input_Attachment = 6,
	
	DescriptorType_Texture = 7,
	DescriptorType_Sampler = 8
};

/*
 * Compare operation
 * Vulkan direct cast
 */
enum CompareOperation
{
	CompareOperation_Never = 0,
	CompareOperation_Less = 1,
	CompareOperation_Equal = 2,
	CompareOperation_LEqual = 3,
	CompareOperation_Greater = 4,
	CompareOperation_NEqual = 5,
	CompareOperation_GEqual = 6,
	CompareOperation_Always = 7
};

/*
 * Stencil Operation
 * Vulkan direct cast
 */
enum StencilTestOperation
{
	StencilTestOperation_Keep = 0,
	StencilTestOperation_Zero = 1,
	StencilTestOperation_Replace = 3,

	StencilTestOperation_Invert = 5
};

/*
 * Cull face
 * Vulkan cast direct
 */
enum CullFace
{
	CullFace_None = 0,
	CullFace_Back = 1,
	CullFace_Front = 2,
	CullFace_Both = 3
};

/*
 * Front Face
 * Vulkan cast direct
 */
enum FrontFace
{
	FrontFace_Clockwise = 0,
	FrontFace_Counter_Clockwise = 1,
};

/*
 * Fill mode
 * Vulkan cast direct
 */
enum FillMode
{
	FillMode_Fill = 0,
	FillMode_Line = 1,
	FillMode_Point = 2
};


/*
 * Color blend function
 * Vulkan direct cast
 */
enum BlenderFactor
{
	BlenderFactor_Zero = 0,
	BlenderFactor_One = 1,
	BlenderFactor_Src_Color = 2,
	BlenderFactor_One_Minus_Src_Color = 3,
	BlenderFactor_Dst_Color = 4,
	BlenderFactor_One_Minus_Dst_Color = 5,
	BlenderFactor_Src_Alpha = 6,
	BlenderFactor_One_Minus_Src_Alpha = 7,
	BlenderFactor_Dst_Alpha = 8,
	BlenderFactor_One_Minus_Dst_Alpha = 9,
};

/*
 * Color blend operation
 * Vulkan direct cast
 */
enum BlendOperation
{
	BlendOperation_Add = 0,
	BlendOperation_Subtract = 1,
	BlendOperation_Reverse_Subtract = 2,
	BlendOperation_Min = 3,
	BlendOperation_Max = 4
};