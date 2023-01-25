#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/gtc/type_ptr.hpp>

#include "OpenRenderRuntime/Core/Basic/RenderMath.h"
#include "OpenRenderRuntime/Core/Render/Render.h"
#include "OpenRenderRuntime/Core/RenderResource/RenderResource.h"
#include "OpenRenderRuntime/Modules/RenderData/Resolvers/MaterialDataResolvers.h"
#include "OpenRenderRuntime/Util/Logger.h"

MaterialBaseDataCreateResolver::MaterialBaseDataCreateResolver(const RenderComponentsData& InData) : RenderSwapDataResolver(InData)
{
}

void MaterialBaseDataCreateResolver::ResolveData(RenderSwapData* Data)
{
	DYNAMIC_CAST_DATA_CHECK(MaterialBaseCreateData, Data, CreateData)
	auto& MaterialBases = ResourcePtr->MaterialBases;
	if(MaterialBases.find(CreateData->MaterialBaseId) != MaterialBases.end())
	{
		LOG_ERROR_FUNCTION("Duplicate material base id");
		delete Data;
		return;
	}

	auto ParamUsage2ShaderStage = [](ParamUsage Usage) -> ShaderStageType
	{
		ShaderStageType BufferShaderStage = 0;
		BufferShaderStage |= Usage & ParamUsageBit_Geometry ? ShaderStageTypeBit_Mesh | ShaderStageTypeBit_Vertex : 0;
		BufferShaderStage |= Usage & ParamUsageBit_Fragment ? ShaderStageTypeBit_Fragment : 0;
		return BufferShaderStage;
	};
	
	std::vector<DescriptorBindingInfo> Bindings;

	//Even if no parameter, a place will provided
	Bindings.push_back({DescriptorType_Uniform_Buffer,  ParamUsage2ShaderStage(CreateData->BufferParamUsageState)});
	for(size_t i=0; i<CreateData->TextureParams.size(); ++i)
	{
		Bindings.push_back({DescriptorType_Texture, ParamUsage2ShaderStage(CreateData->TextureParams[i].Usage)});
	}
	
	RHIDescriptorLayout* Layout = RHIPtr->CreateDescriptorLayout(Bindings);
	if(!Layout)
	{
		LOG_ERROR_FUNCTION("Fail to create material layout");
		delete CreateData;
		return;
	}
	
	RenderMaterialBase* Base = new RenderMaterialBase{
		CreateData->MaterialBaseId,
		Layout,
		CreateData->DoubleSided,
		CreateData->BlendMode,
		{},
		{},
		0,
		{}};

	uint32_t Current = 0;
	for(auto& Param : CreateData->VectorParamNames)
	{
		Base->ParameterBufferMap.emplace(Param, MaterialParameterProxyStruct{Current, MaterialParamType_Vector4Param});
		Current += 4 * sizeof(float);
	}

	for(auto& Param : CreateData->ScalarParamNames)
	{
		Base->ParameterBufferMap.emplace(Param, MaterialParameterProxyStruct{Current, MaterialParamType_ScalarParam});
		Current += sizeof(float);
	}
	
	if(Current)
	{
		Base->UniformBufferSize = ORMath::RoundUp((size_t)Current, 4 * sizeof(float));
	}

	uint32_t Binding = 1;	
	for(auto& Tex : CreateData->TextureParams)
	{
		Base->ParameterTextureBindingMap.emplace(Tex.Name, MaterialTexParamProxyStruct{Tex.Type,Binding});
		++Binding;
	}

	RenderPtr->OnAddNewMaterialBase(CreateData, Base);
	MaterialBases.emplace(CreateData->MaterialBaseId, Base);
	delete CreateData;
}

MaterialInstanceDataCreateResolver::MaterialInstanceDataCreateResolver(const RenderComponentsData& InData) : RenderSwapDataResolver(InData)
{
}

void MaterialInstanceDataCreateResolver::ResolveData(RenderSwapData* Data)
{
	DYNAMIC_CAST_DATA_CHECK(MaterialInstanceCreateData, Data, CreateData)
	auto &MaterialBases = ResourcePtr->MaterialBases;
	auto &Instances = ResourcePtr->Materials;

	if(Instances.find(CreateData->MaterialInstanceId) != Instances.end())
	{
		LOG_ERROR_FUNCTION("Duplicate material instance id");
		delete CreateData;
		return;
	}

	RenderMaterialBase* Base = nullptr;
	if(auto Iter = MaterialBases.find(CreateData->MaterialId); Iter == MaterialBases.end())
	{
		LOG_ERROR_FUNCTION("Invalid base material for material instance");
		delete CreateData;
		return;
	}
	else
	{
		Base = Iter->second;	
	}

	size_t RealSize = Base->UniformBufferSize ? Base->UniformBufferSize : 4 * sizeof(float);
	RHIBuffer* MaterialUniform = RHIPtr->CreateUniformBuffer(RealSize);
	if(!MaterialUniform)
	{
		LOG_ERROR_FUNCTION("Fail to create material instance uniform");
		delete CreateData;
		return;
	}
	
	RHIDescriptorSet *MaterialInstanceSet = RHIPtr->CreateDescriptorSet(Base->MaterialParamLayout);
	if(!MaterialInstanceSet)
	{
		RHIPtr->DestroyBuffer(MaterialUniform);
		LOG_ERROR_FUNCTION("Fail to create material instance descriptor set");
		delete CreateData;
		return;
	}

	RenderMaterialInstance* Instance = new RenderMaterialInstance {CreateData->MaterialInstanceId, Base, MaterialInstanceSet, MaterialUniform};
	std::byte* BufferCPUMem = new std::byte[RealSize];
	for(auto& Param : CreateData->ScalarParams)
	{
		auto Iter = Base->ParameterBufferMap.find(Param.first);
		if(Iter == Base->ParameterBufferMap.end())
		{
			LOG_DEBUG_FUNCTION("Invalid parameter name: {}", Param.first);
		}
		else
		{
			float* MemPtr = reinterpret_cast<float*>(BufferCPUMem + Iter->second.Offset);
			*MemPtr = Param.second;
		}
	}

	for(auto& Param : CreateData->VectorParameters)
	{
		auto Iter = Base->ParameterBufferMap.find(Param.first);
		if(Iter == Base->ParameterBufferMap.end())
		{
			LOG_DEBUG_FUNCTION("Invalid parameter name: {}", Param.first);
		}
		else
		{
			memcpy((BufferCPUMem + Iter->second.Offset), glm::value_ptr(Param.second), 4 * sizeof(float));
		}
	}

	//Transfer memory from cpu to gpu only once
	RHIPtr->SetBufferData(MaterialUniform, BufferCPUMem, RealSize, 0);

	std::vector<TextureWriteInfo> TextureInfo;
	for(auto &Tex : CreateData->MaterialTextures)
	{
		RHITexture* Texture = nullptr;
		auto BindingIter = Base->ParameterTextureBindingMap.find(Tex.first);
		if(BindingIter == Base->ParameterTextureBindingMap.end())
		{
			LOG_DEBUG_FUNCTION("Wrong texture param name: {}", Tex.first);	
		}
		
		auto Iter = ResourcePtr->Textures.find(Tex.second);
		if(Iter == ResourcePtr->Textures.end())
		{
			Texture = ResourcePtr->DefaultResources.DefaultTexture;
		}
		else
		{
			Texture = Iter->second;
		}

		TextureInfo.push_back({Texture, BindingIter->second.Binding});
	}

	RHIPtr->WriteDescriptorSetMulti(MaterialInstanceSet, TextureInfo, {}, {
		{
			MaterialUniform,
			RealSize,
			0,
			DescriptorType_Uniform_Buffer,
			0
		}});

	Instances.emplace(CreateData->MaterialInstanceId, Instance);
}



