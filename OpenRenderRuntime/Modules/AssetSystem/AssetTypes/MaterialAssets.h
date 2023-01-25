#pragma once

#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

#include "OpenRenderRuntime/Modules/AssetSystem/AssetTypes/TextureAssets.h"
#include "OpenRenderRuntime/Core/RHI/RHIEnums.h"
#include "OpenRenderRuntime/Modules/AssetSystem/AssetObject.h"

struct MaterialParamDesc
{
	MaterialParamType Type;

	union
	{
		uint32_t TexBindingIndex;
		uint32_t BufferParamIndex;
	};

	ParamUsage Usage;
};

struct MaterialParamValue
{
	union
	{
		void* Ptr;
		float Value;
	};
	

	MaterialParamType Type;
};

class MaterialParamDescTable
{
	
	std::unordered_map<std::string, MaterialParamDesc> ParamDesc;
	uint32_t ScalarParamCount;
	uint32_t VectorParamCount;
	uint32_t TextureParamCount;

public:

	MaterialParamDescTable();
	MaterialParamDescTable(const MaterialParamDescTable& Table);
	MaterialParamDescTable(MaterialParamDescTable&& Table) noexcept;

	MaterialParamDescTable& operator=(const MaterialParamDescTable& Table);
	MaterialParamDescTable& operator=(MaterialParamDescTable&& Table) noexcept;
	
	const std::unordered_map<std::string, MaterialParamDesc>& GetDesc() const;
	uint32_t GetScalarParamCount() const;
	uint32_t GetVecParamCount() const;
	uint32_t GetTexParamCount() const;

	bool PushScalarParam(const std::string& Name);
	bool PushVectorParam(const std::string& Name);
	bool PushTextureParam(const std::string& Name, const std::string& TypeStr, ParamUsage Usage);
};

class MaterialBaseAsset : public AssetObject
{
	size_t MaterialBaseId;

	/*
	 * Metadata,for param check
	 */
	MaterialParamDescTable ParamDesc;

public:

	MaterialBaseAsset(size_t InMaterialBaseId);

	~MaterialBaseAsset() override;

	void SetParamDesc(MaterialParamDescTable&& InParamDesc);
	
	const MaterialParamDescTable& GetParamTable() const;

	size_t GetMaterialBaseId() const;

	DEFINE_ASSET_TYPE("MaterialBase")
};

class MaterialParamValueTable
{
	std::unordered_map<std::string, MaterialParamValue> ParamValue;
	uint32_t ScalarValueCount;
	uint32_t VectorValueCount;
	uint32_t TextureValueCount;

public:

	MaterialParamValueTable();
	MaterialParamValueTable(const MaterialParamValueTable& Table);
	MaterialParamValueTable(MaterialParamValueTable&& Table) noexcept;

	MaterialParamValueTable& operator=(const MaterialParamValueTable& Table);
	MaterialParamValueTable& operator=(MaterialParamValueTable&& Table) noexcept;

	const std::unordered_map<std::string, MaterialParamValue>& GetValue() const;
	uint32_t GetScalarValueCount() const;
	uint32_t GetVecValueCount() const;
	uint32_t GetTexValueCount() const;
	
	bool SetScalarValue(const std::string& Name, float Value);
	bool SetVectorValue(const std::string& Name, const glm::vec4& Value);
	bool SetTextureValue(const std::string& Name, TextureAsset* Value, MaterialParamType TextureParamType);
	
};

class MaterialInstanceAsset : public AssetObject
{
	size_t MaterialInstanceId;

	MaterialBaseAsset* BasePtr;

	/*
	 * This meta data for current dynamic material
	 */
	MaterialParamValueTable ParamValues; 

public:

	MaterialInstanceAsset(size_t InId, MaterialBaseAsset* InBase);

	~MaterialInstanceAsset() override;

	void SetParamValues(MaterialParamValueTable&& Params);

	const MaterialParamValueTable& GetValueTable() const;

	size_t GetMaterialInstanceId() const;

	MaterialBaseAsset* GetBase() const;
	
	DEFINE_ASSET_TYPE("MaterialInstance")
};