#pragma once
#include <unordered_map>
#include <vector>
#include <string>
#include "OpenRenderRuntime/Core/RHI/RHIDescriptor.h"
#include "OpenRenderRuntime/Core/RHI/RHIPipeline.h"
#include "OpenRenderRuntime/Util/AutoIncreaseIdAllocator.h"


struct MaterialParameterProxyStruct
{
	size_t Offset = 0;
	MaterialParamType Type = MaterialParamType_ScalarParam;
};

struct MaterialTexParamProxyStruct
{
	TextureType Type = TextureType_2D;
	uint32_t Binding = 0;
};

class RenderMaterialBase
{

public:

	inline static AutoIncreaseIdAllocator Registry {0};
	constexpr static size_t BAD_MATERIAL_BASE_ID = SIZE_MAX;

	size_t Id = 0;
	RHIDescriptorLayout* MaterialParamLayout = nullptr;

	bool DoubleSided = false;
	PipelineBlendMode BlendMode = PipelineBlendMode_Opaque;

	std::vector<RHIPipeline*> Pipelines {}; //Each material render pass will get a slot

	std::unordered_map<std::string, MaterialParameterProxyStruct> ParameterBufferMap {};
	size_t UniformBufferSize = 0;
	std::unordered_map<std::string, MaterialTexParamProxyStruct> ParameterTextureBindingMap {};
	
};
