#pragma once
#include "RHIBuffer.h"
#include "OpenRenderRuntime/Core/RHI/RHIEnums.h"
#include "OpenRenderRuntime/Core/RHI/RHITexture.h"

struct DescriptorBindingInfo
{
	DescriptorType Type = DescriptorType_Uniform_Buffer;
	ShaderStageType UsageStage = ShaderStageTypeBit_Fragment;
};

class RHIDescriptorLayout
{
	
public:
	
};

class RHIDescriptorSet
{
	
public:
	
};

struct TextureWriteInfo
{
	RHITexture* Texture = nullptr;
	uint32_t Binding = 0;
};

struct ImageWriteInfo
{
	RHITexImageView* ImageView = nullptr;
	DescriptorType ImageType = DescriptorType_Storage_Image; //Input attachment or storage image
	TexImageLayout ImageLayout = TexImageLayout_Shader_TexImage; //If write, use general
	uint32_t Binding = 0;
};

struct BufferWriteInfo
{
	RHIBuffer* Buffer = nullptr;
	size_t Range = 0;
	size_t Offset = 0;
	DescriptorType BufferType = DescriptorType_Uniform_Buffer; //Uniform buffer or storage buffer
	uint32_t Binding = 0;
};