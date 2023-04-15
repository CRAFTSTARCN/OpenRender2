#pragma once
#include "RHIBuffer.h"
#include "OpenRenderRuntime/Core/RHI/RHIEnums.h"
#include "OpenRenderRuntime/Core/RHI/RHITexture.h"
#include "OpenRenderRuntime/Core/RHI/RHISampler.h"

struct DescriptorBindingInfo
{
	DescriptorType Type = DescriptorType_Uniform_Buffer;
	ShaderStageType UsageStage = ShaderStageTypeBit_Fragment;
};

struct BindlessDescriptorInfo
{
	DescriptorType Type = DescriptorType_Texture_With_Sampler;
	ShaderStageType UsageStage = ShaderStageTypeBit_Fragment;
	uint32_t CountHint = 0;
};

class RHIDescriptorLayout
{
	
public:
	
};

class RHIDescriptorSet
{
	
public:
	
};

struct TextureWithSamplerWriteInfo
{
	RHITextureView* Texture = nullptr;
	RHISampler* Sampler = nullptr;
	uint32_t Binding = 0;
	uint32_t Index = 0;
};

struct ImageWriteInfo
{
	RHITextureView* ImageView = nullptr;
	DescriptorType ImageType = DescriptorType_Storage_Image; //Input attachment or storage image
	TexImageLayout ImageLayout = TexImageLayout_Shader_TexImage; //If write, use general
	uint32_t Binding = 0;
	uint32_t Index = 0;
};

struct BufferWriteInfo
{
	RHIBuffer* Buffer = nullptr;
	size_t Range = 0;
	size_t Offset = 0;
	DescriptorType BufferType = DescriptorType_Uniform_Buffer; //Uniform buffer or storage buffer
	uint32_t Binding = 0;
	uint32_t Index = 0;
};