#pragma once

#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <unordered_map>

#include "OpenRenderRuntime/Core/RHI/RHIEnums.h"
#include "OpenRenderRuntime/Modules/RenderData/RenderSwapData.h"


struct MaterialBaseShaderData
{
	std::vector<std::byte> MeshShaderData;
	std::vector<std::byte> FragmentShaderData;
};

struct MaterialCreateTexParamStruct
{
	std::string Name;
	ParamUsage Usage = 0;
	TextureType Type = TextureType_2D;
};

class MaterialBaseCreateData : public RenderSwapData
{
	
public:

	size_t MaterialBaseId = 0;
	PipelineBlendMode BlendMode = PipelineBlendMode_Opaque;
	bool DoubleSided = false;

	std::unordered_map<std::string, MaterialBaseShaderData> ShaderDataTable {};
	std::vector<std::string> ScalarParamNames {};
	std::vector<std::string> VectorParamNames {};

	ParamUsage BufferParamUsageState = ParamUsageBit_Fragment;
	
	std::vector<MaterialCreateTexParamStruct> TextureParams {};
	
	
	DEFINE_SWAP_DATA_TYPE(MaterialBaseCreateData)
};

class MaterialBaseDestroyData : public RenderSwapData
{
	
public:

	size_t MaterialBaseId = 0;

	DEFINE_SWAP_DATA_TYPE(MaterialBaseDestroyData)
};

class MaterialInstanceCreateData : public RenderSwapData
{
	
public:

	size_t MaterialInstanceId = 0;
	size_t MaterialId = 0;

	std::vector<std::pair<std::string, float>> ScalarParams {};
	std::vector<std::pair<std::string, glm::vec4>> VectorParameters;
	std::vector<std::pair<std::string, size_t>> MaterialTextures {};
	
	DEFINE_SWAP_DATA_TYPE(MaterialInstanceCreateData)
};

class MaterialInstanceDestroyData : public RenderSwapData
{
	
public:

	size_t MaterialInstanceId = 0;

	DEFINE_SWAP_DATA_TYPE(MaterialInstanceDestroyData)
};

class DefaultMaterialData : public RenderSwapData
{
	
public:
	size_t DefaultMaterialId = 0;

	DEFINE_SWAP_DATA_TYPE(DefaultMaterialData)
};