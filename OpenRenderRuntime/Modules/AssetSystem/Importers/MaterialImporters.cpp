#include "OpenRenderRuntime/Modules/AssetSystem/Importers/MaterialImporters.h"
#include "OpenRenderRuntime/Modules/AssetSystem/AssetTypes/MaterialAssets.h"

#include <filesystem>
#include <fstream>

#include "OpenRenderRuntime/Core/RenderResource/RenderMaterialBase.h"
#include "OpenRenderRuntime/Core/RenderResource/RenderMaterialInstance.h"
#include "OpenRenderRuntime/Core/RenderResource/RenderResource.h"
#include "OpenRenderRuntime/Util/Logger.h"

std::vector<std::byte> MaterialBaseImporter::LoadShader2Byte(const std::string& RelPath)
{
	std::string FullPath = (std::filesystem::path(ConfigPtr->BasePath) / RelPath).generic_string()
	                       + "." + ConfigPtr->ShaderPlatform;
	std::ifstream In;
	std::vector<std::byte> Bytes;
	In.open(FullPath, std::ios::in|std::ios::binary);

	if(!In.is_open())
	{
		LOG_ERROR_FUNCTION("Cannot open shader file for pass, given path: {0} platform {1}", RelPath, ConfigPtr->ShaderPlatform);
		return {};
	}
	
	In.seekg(0, std::ios::end);
	std::streamsize Len = In.tellg();
	In.seekg(0, std::ios::beg);

	Bytes.resize(Len);
	In.read((char*)Bytes.data(), Len);
	In.close();
	
	return Bytes;
}

MaterialBaseShaderData MaterialBaseImporter::AnalShaderData(Json PassJson)
{
	MaterialBaseShaderData Data {};
	const std::string& MeshShaderPath = PassJson["MeshShader"].string_value();
	if(MeshShaderPath.empty())
	{
		LOG_ERROR_FUNCTION("Empty mesh shader path");
	}
	else
	{
		Data.MeshShaderData = LoadShader2Byte(MeshShaderPath);
	}

	const std::string& FragShaderPath = PassJson["FragmentShader"].string_value();
	if(FragShaderPath.empty())
	{
		LOG_ERROR_FUNCTION("Empty fragment shader path");
	}
	else
	{
		Data.FragmentShaderData = LoadShader2Byte(FragShaderPath);
	}

	return Data;
}

PipelineBlendMode MaterialBaseImporter::AnaBlendMode(Json Asset)
{
	static std::unordered_map<std::string, PipelineBlendMode> BlendModeMap {
		{"Opaque", PipelineBlendMode_Opaque},
		{"Masked", PipelineBlendMode_Masked},
		{"Translucent", PipelineBlendMode_Translucent},
		{"Additive", PipelineBlendMode_Additive}
	};

	const std::string& BlendModeStr = Asset["BlendMode"].string_value();
	auto Iter = BlendModeMap.find(BlendModeStr);
	if(Iter == BlendModeMap.end())
	{
		LOG_ERROR_FUNCTION("Unrecognized blend mode {0} , opaque will used", BlendModeStr.c_str());
		return PipelineBlendMode_Opaque;
	}

	return Iter->second;
}

std::pair<MaterialParamDescTable, bool> MaterialBaseImporter::AnaParamDesc(Json Asset)
{
	MaterialParamDescTable ParamTable;
	Json BufferParams = Asset["BufferParameters"];
	auto&& VecParams = BufferParams["Vec"].array_items();
	for(auto&& Name : VecParams)
	{
		bool Success = ParamTable.PushVectorParam(Name.string_value());
		if(!Success)
		{
			LOG_ERROR_FUNCTION("Param {0} fail to push, fatal", Name.string_value().c_str());
			return std::make_pair(std::move(ParamTable), false);
		}
	}

	auto&& ScalarParams = BufferParams["Scalar"].array_items();

	for(auto&& Name : ScalarParams)
	{
		bool Success = ParamTable.PushScalarParam(Name.string_value());
		if(!Success)
		{
			LOG_ERROR_FUNCTION("Param {0} fail to push, fatal", Name.string_value().c_str());
			return std::make_pair(std::move(ParamTable), false);
		}
	}

	auto&& TexParam = Asset["TexParam"].array_items();
	for(auto&& TexDesc : TexParam)
	{
		const std::string& NameStr = TexDesc["Name"].string_value();
		ParamUsage Usage = AnaParamUsage(TexDesc["Stage"].array_items());
		bool Success = ParamTable.PushTextureParam(NameStr, TexDesc["Type"].string_value(), Usage);
		if(!Success)
		{
			LOG_ERROR_FUNCTION("Param {0} fail to push, fatal", NameStr.c_str());
			return std::make_pair(std::move(ParamTable), false);
		}
	}
	
	return std::make_pair(std::move(ParamTable), true);
}

void MaterialBaseImporter::SendDestroy2Swap(size_t MaterialBaseId)
{
	MaterialBaseDestroyData* DestroyData = new MaterialBaseDestroyData;
	DestroyData->MaterialBaseId = MaterialBaseId;
	SwapDataCenterPtr->GetLogicSide().push_back(DestroyData);
}


MaterialBaseImporter::MaterialBaseImporter(const AssetImportData& InData) : AssetImporter(InData)
{
}

MaterialBaseImporter::~MaterialBaseImporter()
{
}

size_t MaterialBaseImporter::LoadAsset(Json AssetJson, const std::string& RelPath)
{
	std::unordered_map<std::string, MaterialBaseShaderData> ShaderData;
	
	auto PassShaders = AssetJson["PassShaders"].object_items();
	for(auto &Pass : PassShaders)
	{
		MaterialBaseShaderData PassShaderData = AnalShaderData(Pass.second);
		if(PassShaderData.FragmentShaderData.empty() || PassShaderData.MeshShaderData.empty())
		{
			LOG_ERROR_FUNCTION("No shader for pass {0} in {1}", Pass.first.c_str(), RelPath.c_str());
		}
		else
		{
			ShaderData.emplace(Pass.first, std::move(PassShaderData));	
		}
	}

	bool DoubleSided = AssetJson["DoubleSided"].bool_value();
	PipelineBlendMode BlendMode = AnaBlendMode(AssetJson);
	auto &&[ParamTable, Success] = AnaParamDesc(AssetJson);
	if(!Success)
	{
		LOG_ERROR_FUNCTION("Fail to parse parameter, fatal, asset : {0}", RelPath.c_str());
	}

	ParamUsage BufferUsage = AssetImporter::AnaParamUsage(AssetJson["BufferParameters"]["BufferStage"].array_items());

	size_t Id = RenderMaterialBase::Registry.GetNewId();
	MaterialBaseAsset* NewAsset = new MaterialBaseAsset(Id);
	NewAsset->SetParamDesc(std::move(ParamTable));

	size_t AssetId = RegistryPtr->RegisterNew(NewAsset, RelPath);
	if(AssetId == AssetRegistry::BAD_GASSET_ID)
	{
		delete NewAsset;
		return AssetRegistry::BAD_GASSET_ID;
	}

	std::vector<std::string> ScalarParamList(NewAsset->GetParamTable().GetScalarParamCount());
	std::vector<std::string> VectorParamList(NewAsset->GetParamTable().GetVecParamCount()); 
	std::vector<MaterialCreateTexParamStruct> TextureParamList(NewAsset->GetParamTable().GetTexParamCount());

	for(auto &[Name, Desc] : NewAsset->GetParamTable().GetDesc())
	{
		if(Desc.Type == MaterialParamType_ScalarParam)
		{
			ScalarParamList[Desc.BufferParamIndex] = Name;
		}
		else if(Desc.Type == MaterialParamType_Vector4Param)
		{
			VectorParamList[Desc.BufferParamIndex] = Name;
		}
		else if(Desc.Type == MaterialParamType_Texture)
		{
			TextureParamList[Desc.TexBindingIndex].Name = Name;
			TextureParamList[Desc.TexBindingIndex].Type = TextureType_2D;
			TextureParamList[Desc.TexBindingIndex].Usage = Desc.Usage;
		}
		else if(Desc.Type == MaterialParamType_Texture_Cube)
		{
			TextureParamList[Desc.TexBindingIndex].Name = Name;
			TextureParamList[Desc.TexBindingIndex].Type = TextureType_Cube;
			TextureParamList[Desc.TexBindingIndex].Usage = Desc.Usage;
		}
	}

	MaterialBaseCreateData* SwapData = new MaterialBaseCreateData;
	SwapData->MaterialBaseId = Id;
	SwapData->BlendMode = BlendMode;
	SwapData->DoubleSided = DoubleSided;
	SwapData->ShaderDataTable = std::move(ShaderData);
	SwapData->ScalarParamNames = std::move(ScalarParamList);
	SwapData->VectorParamNames = std::move(VectorParamList);
	SwapData->BufferParamUsageState = BufferUsage;
	SwapData->TextureParams = std::move(TextureParamList);

	SwapDataCenterPtr->GetLogicSide().push_back(SwapData);
	return AssetId;
}

void MaterialBaseImporter::UnloadAsset(AssetObject* Asset)
{
	DYNAMIC_CAST_ASSET_CHECK(MaterialBaseAsset, Asset, MaterialBase)
	SendDestroy2Swap(MaterialBase->GetMaterialBaseId());
	RegistryPtr->Unregister(MaterialBase->GetId());
	delete MaterialBase;
}

MaterialParamValueTable MaterialInstanceImporter::AnaParamValue(const MaterialBaseAsset* Base, Json Params)
{
	MaterialParamValueTable Table;
	for(auto&& [Name, Desc] : Base->GetParamTable().GetDesc())
	{
		Json Param = Params[Name];
		if(Desc.Type == MaterialParamType_ScalarParam)
		{
			ImportScalarParameter(Table, Name, Desc, Param);
		}
		else if (Desc.Type == MaterialParamType_Vector4Param)
		{
			ImportVectorParameter(Table, Name, Desc, Param);
		}
		else if (Desc.Type == MaterialParamType_Texture || Desc.Type == MaterialParamType_Texture_Cube)
		{
			ImportTextureParameter(Table, Name, Desc, Param);	
		}
	}
	return Table;
}

void MaterialInstanceImporter::SendDestroy2Swap(size_t MaterialInstanceId)
{
	MaterialInstanceDestroyData* DestroyData = new MaterialInstanceDestroyData;
	DestroyData->MaterialInstanceId = MaterialInstanceId;
	SwapDataCenterPtr->GetLogicSide().push_back(DestroyData);
}

void MaterialInstanceImporter::ImportScalarParameter(MaterialParamValueTable& Table, const std::string& Name,
                                                     const MaterialParamDesc& Desc, Json Param)
{
	if(Param.is_number())
	{
		Table.SetScalarValue(Name, (float)Param.number_value());
	}
	else
	{
		LOG_ERROR_FUNCTION("Missing or error type of Scalar value, Param: {0}", Name);
		Table.SetScalarValue(Name, 0);
	}
}

void MaterialInstanceImporter::ImportVectorParameter(MaterialParamValueTable& Table, const std::string& Name,
	const MaterialParamDesc& Desc, Json Param)
{
	auto AnaComponent = [&](const std::string& Component)
	{
		Json Comp = Param[Component];
		if(Comp.is_number())
		{
			return (float)Comp.number_value();
		}

		LOG_ERROR_FUNCTION("Vector param {0} missing component {1}, will use 0", Name.c_str(), Component.c_str());
		return 0.0f;
	};

	glm::vec4 Val {};
	Val.r = AnaComponent("R");
	Val.g = AnaComponent("G");
	Val.b = AnaComponent("B");
	Val.a = AnaComponent("A");

	Table.SetVectorValue(Name, Val);
}

void MaterialInstanceImporter::ImportTextureParameter(MaterialParamValueTable& Table, const std::string& Name,
	const MaterialParamDesc& Desc, Json Param)
{
	if(!Param.is_string())
	{
		LOG_ERROR_FUNCTION("Missing of inavlid type for param {0}, null ptr will used", Name.c_str());
		Table.SetTextureValue(Name, nullptr, Desc.Type);
	}

	AssetObject* TexAssetObj = RegistryPtr->TryGetObjectByPath(Param.string_value());

	if(!TexAssetObj)
	{
		size_t Id = OnCascadeLoading(Param.string_value());
		if(Id == AssetRegistry::BAD_GASSET_ID)
		{
			LOG_ERROR_FUNCTION("Fail to load {0}", Param.string_value().c_str());
			Table.SetTextureValue(Name, nullptr, Desc.Type);
			return;
		}

		TexAssetObj = RegistryPtr->GetObjectById(Id);
	}

	if(Desc.Type == MaterialParamType_Texture_Cube)
	{
		TextureCubeAsset* TexCube = dynamic_cast<TextureCubeAsset*>(TexAssetObj);
		if(!TexCube)
		{
			LOG_ERROR_FUNCTION("Invalid asset type, require texture cube, {0} provided", TexAssetObj->GetAssetTypeName().c_str());
		}
		else
		{
			TexCube->IncreaseUsageCount();
		}
		Table.SetTextureValue(Name, TexCube, Desc.Type);
	}
	else
	{
		Texture2DAsset* Tex2D = dynamic_cast<Texture2DAsset*>(TexAssetObj);
		if(!Tex2D)
		{
			LOG_ERROR_FUNCTION("Invalid asset type, require texture 2d, {0} provided", Tex2D->GetAssetTypeName().c_str());
		}
		else
		{
			Tex2D->IncreaseUsageCount();
		}
		Table.SetTextureValue(Name, Tex2D, Desc.Type);
	}
}

MaterialInstanceImporter::MaterialInstanceImporter(const AssetImportData& InData) : AssetImporter(InData)
{
}

MaterialInstanceImporter::~MaterialInstanceImporter()
{
}

size_t MaterialInstanceImporter::LoadAsset(Json AssetJson, const std::string& RelPath)
{
	const std::string& BaseMaterialPath = AssetJson["Base"].string_value();
	AssetObject* BaseMaterial = RegistryPtr->TryGetObjectByPath(BaseMaterialPath);
	if(!BaseMaterial)
	{
		size_t BaseId = OnCascadeLoading(BaseMaterialPath);
		if(BaseId == AssetRegistry::BAD_GASSET_ID)
		{
			LOG_ERROR_FUNCTION("No valid and fail to import base material for {0}, fatal", RelPath.c_str());
			return AssetRegistry::BAD_GASSET_ID;
		}
		BaseMaterial = RegistryPtr->GetObjectById(BaseId);
	}

	MaterialBaseAsset* Base = dynamic_cast<MaterialBaseAsset*>(BaseMaterial);
	if(!Base)
	{
		LOG_ERROR_FUNCTION("Invalid base type, require material base, provided {0}", BaseMaterial->GetAssetTypeName());
		return AssetRegistry::BAD_GASSET_ID;
	}

	Base->IncreaseUsageCount();

	MaterialParamValueTable ValueTable = AnaParamValue(Base, AssetJson["Params"]);

	size_t MaterialInstanceId = RenderMaterialInstance::Registry.GetNewId();
	MaterialInstanceAsset* NewAsset = new MaterialInstanceAsset(MaterialInstanceId, Base);
	NewAsset->SetParamValues(std::move(ValueTable));
	size_t AssetId = RegistryPtr->RegisterNew(NewAsset, RelPath);

	if(AssetId == AssetRegistry::BAD_GASSET_ID)
	{
		delete NewAsset;
		return AssetRegistry::BAD_GASSET_ID;
	}

	
	std::vector<std::pair<std::string, float>> ScalarParams {};
	std::vector<std::pair<std::string, glm::vec4>> VectorParameters;
	std::vector<std::pair<std::string, size_t>> MaterialTextures {};

	for(auto && [Name, Val] : NewAsset->GetValueTable().GetValue())
	{
		if(Val.Type == MaterialParamType_ScalarParam)
		{
			ScalarParams.push_back(std::make_pair(Name, Val.Value));
		}
		else if(Val.Type == MaterialParamType_Vector4Param)
		{
			VectorParameters.push_back(std::make_pair(Name, *((const glm::vec4*)Val.Ptr)));
		}
		else if(Val.Type == MaterialParamType_Texture || Val.Type == MaterialParamType_Texture_Cube)
		{
			if(Val.Ptr == nullptr)
			{
				MaterialTextures.emplace_back(std::make_pair(Name, RenderResource::BAD_TEXTURE_ID));
			}
			else
			{
				MaterialTextures.emplace_back(std::make_pair(Name, ((const TextureAsset*)Val.Ptr)->GetTexId()));
			}
		}
	}

	MaterialInstanceCreateData* InstanceCreateData = new MaterialInstanceCreateData;
	InstanceCreateData->MaterialId = Base->GetMaterialBaseId();
	InstanceCreateData->MaterialInstanceId = MaterialInstanceId;
	InstanceCreateData->ScalarParams = std::move(ScalarParams);
	InstanceCreateData->VectorParameters = std::move(VectorParameters);
	InstanceCreateData->MaterialTextures = std::move(MaterialTextures);

	SwapDataCenterPtr->GetLogicSide().push_back(InstanceCreateData);

	return AssetId;
}

void MaterialInstanceImporter::UnloadAsset(AssetObject* Asset)
{
	DYNAMIC_CAST_ASSET_CHECK(MaterialInstanceAsset, Asset, MaterialInstance)
	SendDestroy2Swap(MaterialInstance->GetMaterialInstanceId());
	for(auto && [ValueName, Value] : MaterialInstance->GetValueTable().GetValue())
	{
		if(Value.Type == MaterialParamType_Texture || Value.Type == MaterialParamType_Texture_Cube)
		{
			TextureAsset* RefTex = reinterpret_cast<TextureAsset*>(Value.Ptr);
			RefTex->DecreaseUsageCount();
		}
	}
	MaterialInstance->GetBase()->DecreaseUsageCount();
	
	RegistryPtr->Unregister(MaterialInstance->GetId());
	delete MaterialInstance;
}

