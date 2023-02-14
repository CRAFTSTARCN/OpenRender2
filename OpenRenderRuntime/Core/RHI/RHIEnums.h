#pragma once
#include <cstdint>
/*
 * Buffer type
 */
enum BufferTypeBit
{
	BufferTypeBit_Vertex  = 1,
	BufferTypeBit_Index   = 1 << 1,
	BufferTypeBit_Storage = 1 << 2,
	BufferTypeBit_Uniform = 1 << 3
};
typedef uint32_t BufferTypeFlag;

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
	MaterialParamType_Vector4Param,
	MaterialParamType_ScalarParam,
	MaterialParamType_Texture,
	MaterialParamType_Texture_Cube
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
 * Render attachment type
 */
enum RenderImageAttachmentType
{

	RenderImageAttachmentType_Color_Attachment = 0,
	RenderImageAttachmentType_Depth_Attachment = 1,
    
	RenderImageAttachmentType_Post_Process_ImageBuffer = 2,
    
	RenderImageAttachmentType_SwapChain = 3,

	RenderImageAttachmentType_Depth_Stencil_Attachment = 4,

	RnederImageAttachmentType_ComputeStorage = 5
};

/*
 * Image usage
 */
enum ImageExtraUsageBit
{
	ImageExtraUsageBit_Sample          = 1,
	ImageExtraUsageBit_Storage_Image   = 1 << 1,
	ImageExtraUsageBit_Transfer_Dst    = 1 << 2,
	ImageExtraUsageBit_Transfer_Src    = 1 << 3,
	ImageExtraUsageBit_InputAttachment = 1 << 4,
	ImageExtraUsageBit_TransientInputAttachment = 1 << 5


};
typedef uint32_t ImageExtraUsage;

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
 * Direct vulkan cast
 */
enum TexImageLayout
{
	TexImageLayout_Undefined = 0,
	TexImageLayout_General = 1,
	TexImageLayout_Color_Attachment = 2,
	TexImageLayout_Depth_Stencil_Attachment = 3,
	TexImageLayout_Depth_Stencil_Readonly = 4,
	TexImageLayout_Shader_TexImage = 5,
	TexImageLayout_Transfer_Src = 6,
	TexImageLayout_Transfer_Dst = 8,

	TexImageLayout_For_Present = 1000001002
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
enum TexturePixelFormat
{
	TexturePixelFormat_RGB8 = 0,
	TexturePixelFormat_RGB8_SRGB = 1,
	TexturePixelFormat_RGB32_FLOAT = 2,
    
	TexturePixelFormat_RGBA8 = 3,
	TexturePixelFormat_RGBA8_SRGB = 4,
	TexturePixelFormat_RGBA32_FLOAT = 5,
    
	TexturePixelFormat_R32_FLOAT = 6,
	TexturePixelFormat_D32_FLOAT = 7,

	TexturePixelFormat_BGRA8 = 8,
	TexturePixelFormat_BGRA8_SRGB = 9,
	TexturePixelFormat_RGBA16_FLOAT = 10,
	TexturePixelFormat_A2_BGR10_HDR = 11
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
	DescriptorType_Texture = 0, //Represent texture and sampler
	DescriptorType_Storage_Image = 1,
	DescriptorType_Uniform_Buffer = 2,
	DescriptorType_Storage_Buffer = 3,
	DescriptorType_Uniform_Buffer_Dynamic = 4,
	DescriptorType_Storage_Buffer_Dynamic = 5,
	DescriptorType_Input_Attachment = 6,
	
	DescriptorType_Texture_Image = 7,
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