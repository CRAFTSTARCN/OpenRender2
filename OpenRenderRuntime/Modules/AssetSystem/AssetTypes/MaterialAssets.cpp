#include "OpenRenderRuntime/Modules/AssetSystem/AssetTypes/MaterialAssets.h"

#include "OpenRenderRuntime/Util/Logger.h"

MaterialParamDescTable::MaterialParamDescTable() : ScalarParamCount(0), VectorParamCount(0), TextureParamCount(0)
{
}

MaterialParamDescTable::MaterialParamDescTable(const MaterialParamDescTable& Table) :
	ParamDesc(Table.ParamDesc),
	ScalarParamCount(Table.ScalarParamCount),
	VectorParamCount(Table.VectorParamCount),
	TextureParamCount(Table.TextureParamCount)
{
}

MaterialParamDescTable::MaterialParamDescTable(MaterialParamDescTable&& Table) noexcept :
	ParamDesc(std::move(Table.ParamDesc)),
	ScalarParamCount(Table.ScalarParamCount),
	VectorParamCount(Table.VectorParamCount),
	TextureParamCount(Table.TextureParamCount)
{
}

MaterialParamDescTable& MaterialParamDescTable::operator=(const MaterialParamDescTable& Table)
{
	this->ParamDesc = Table.ParamDesc;
	this->ScalarParamCount = Table.ScalarParamCount;
	this->VectorParamCount = Table.VectorParamCount;
	this->TextureParamCount = Table.TextureParamCount;
	return *this;
}

MaterialParamDescTable& MaterialParamDescTable::operator=(MaterialParamDescTable&& Table) noexcept
{
	this->ParamDesc = std::move(Table.ParamDesc);
	this->ScalarParamCount = Table.ScalarParamCount;
	this->VectorParamCount = Table.VectorParamCount;
	this->TextureParamCount = Table.TextureParamCount;
	return *this;
}

const std::unordered_map<std::string, MaterialParamDesc>& MaterialParamDescTable::GetDesc() const
{
	return ParamDesc;
}

uint32_t MaterialParamDescTable::GetScalarParamCount() const
{
	return ScalarParamCount;
}

uint32_t MaterialParamDescTable::GetVecParamCount() const
{
	return VectorParamCount;
}

uint32_t MaterialParamDescTable::GetTexParamCount() const
{
	return TextureParamCount;
}

bool MaterialParamDescTable::PushScalarParam(const std::string& Name)
{
	if(Name.empty())
	{
		LOG_ERROR_FUNCTION("Empty name not allowed");
		return false;
	}

	auto &&[Iter, Success] = ParamDesc.try_emplace(
		Name,
		MaterialParamDesc{MaterialParamType_ScalarParam, {ScalarParamCount}, 0});
	if(!Success)
	{
		LOG_ERROR_FUNCTION("Duplicate name might pushed");
		return false;
	}

	++ScalarParamCount;
	return true;
}

bool MaterialParamDescTable::PushVectorParam(const std::string& Name)
{
	if(Name.empty())
	{
		LOG_ERROR_FUNCTION("Empty name not allowed");
		return false;
	}

	auto &&[Iter, Success] = ParamDesc.try_emplace(
		Name,
		MaterialParamDesc{MaterialParamType_Vector4Param, {VectorParamCount}, 0});
	if(!Success)
	{
		LOG_ERROR_FUNCTION("Duplicate name might pushed");
		return false;
	}

	++VectorParamCount;
	return true;
}

bool MaterialParamDescTable::PushTextureParam(const std::string& Name, const std::string& TypeStr, ParamUsage Usage)
{
	static std::unordered_map<std::string, MaterialParamType> TypeMap {
		{"Tex2D", MaterialParamType_Texture},
		{"TexCube", MaterialParamType_Texture_Cube}
	};

	auto IterType = TypeMap.find(TypeStr);
	if(IterType == TypeMap.end())
	{
		LOG_ERROR_FUNCTION("Invalid texture type");
		return false;
	}

	auto &&[Iter, Success] = ParamDesc.try_emplace(
		Name,
		MaterialParamDesc{IterType->second, {TextureParamCount}, Usage});
	if(!Success)
	{
		LOG_ERROR_FUNCTION("Duplicate name might pushed");
		return false;
	}

	++TextureParamCount;
	return true;
}

MaterialBaseAsset::MaterialBaseAsset(size_t InMaterialBaseId) : MaterialBaseId(InMaterialBaseId)
{
}

MaterialBaseAsset::~MaterialBaseAsset()
{
}

void MaterialBaseAsset::SetParamDesc(MaterialParamDescTable&& InParamDesc)
{
	ParamDesc = std::move(InParamDesc);
}

const MaterialParamDescTable& MaterialBaseAsset::GetParamTable() const
{
	return ParamDesc;
}

size_t MaterialBaseAsset::GetMaterialBaseId() const
{
	return MaterialBaseId;
}

MaterialParamValueTable::MaterialParamValueTable() : ScalarValueCount(0), VectorValueCount(0), TextureValueCount(0)
{
	
}

MaterialParamValueTable::MaterialParamValueTable(const MaterialParamValueTable& Table) :
	ParamValue(Table.ParamValue),
	ScalarValueCount(Table.ScalarValueCount),
	VectorValueCount(Table.VectorValueCount),
	TextureValueCount(Table.TextureValueCount)
{
}

MaterialParamValueTable::MaterialParamValueTable(MaterialParamValueTable&& Table) noexcept :
	ParamValue(std::move(Table.ParamValue)),
	ScalarValueCount(Table.ScalarValueCount),
	VectorValueCount(Table.VectorValueCount),
	TextureValueCount(Table.TextureValueCount)
{
}

MaterialParamValueTable& MaterialParamValueTable::operator=(const MaterialParamValueTable& Table)
{
	this->ParamValue = Table.ParamValue;
	this->ScalarValueCount = Table.ScalarValueCount;
	this->VectorValueCount = Table.VectorValueCount;
	this->TextureValueCount = Table.TextureValueCount;

	return *this;
}

MaterialParamValueTable& MaterialParamValueTable::operator=(MaterialParamValueTable&& Table) noexcept
{
	this->ParamValue = std::move(Table.ParamValue);
	this->ScalarValueCount = Table.ScalarValueCount;
	this->VectorValueCount = Table.VectorValueCount;
	this->TextureValueCount = Table.TextureValueCount;

	return *this;
}

const std::unordered_map<std::string, MaterialParamValue>& MaterialParamValueTable::GetValue() const
{
	return ParamValue;
}

uint32_t MaterialParamValueTable::GetScalarValueCount() const
{
	return  ScalarValueCount;
}

uint32_t MaterialParamValueTable::GetVecValueCount() const
{
	return VectorValueCount;
}

uint32_t MaterialParamValueTable::GetTexValueCount() const
{
	return TextureValueCount;
}

bool MaterialParamValueTable::SetScalarValue(const std::string& Name, float Value)
{
	MaterialParamValue ValueData {};
	ValueData.Value = Value;
	ValueData.Type = MaterialParamType_ScalarParam;
	
	auto&& [Iter, Success] = ParamValue.try_emplace(Name, ValueData);
	return Success;
}

bool MaterialParamValueTable::SetVectorValue(const std::string& Name, const glm::vec4& Value)
{
	MaterialParamValue ValueData {};
	ValueData.Ptr = new glm::vec4(Value);
	ValueData.Type = MaterialParamType_Vector4Param;
	
	auto&& [Iter, Success] = ParamValue.try_emplace(Name, ValueData);
	return Success;
}

bool MaterialParamValueTable::SetTextureValue(const std::string& Name, TextureAsset* Value, MaterialParamType TextureParamType)
{
	if(TextureParamType != MaterialParamType_Texture && TextureParamType != MaterialParamType_Texture_Cube)
	{
		LOG_ERROR_FUNCTION("Error type: not a texture type pass to this function");
		return false;
	}
	MaterialParamValue ValueData {};
	ValueData.Ptr = Value;
	ValueData.Type = TextureParamType;
	
	auto&& [Iter, Success] = ParamValue.try_emplace(Name, ValueData);
	return Success;
}

MaterialInstanceAsset::MaterialInstanceAsset(size_t InId, MaterialBaseAsset* InBase) : MaterialInstanceId(InId), BasePtr(InBase)
{
}

MaterialInstanceAsset::~MaterialInstanceAsset()
{
}

void MaterialInstanceAsset::SetParamValues(MaterialParamValueTable&& Params)
{
	ParamValues = std::move(Params);
}

const MaterialParamValueTable& MaterialInstanceAsset::GetValueTable() const
{
	return ParamValues;
}

size_t MaterialInstanceAsset::GetMaterialInstanceId() const
{
	return MaterialInstanceId;
}

MaterialBaseAsset* MaterialInstanceAsset::GetBase() const
{
	return BasePtr;
}
