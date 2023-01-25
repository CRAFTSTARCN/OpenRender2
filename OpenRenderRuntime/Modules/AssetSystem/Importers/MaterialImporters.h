#pragma once
#include "OpenRenderRuntime/Modules/AssetSystem/AssetImporter.h"
#include "OpenRenderRuntime/Modules/RenderData/MaterialSwapData.h"

class MaterialParamDescTable;
class MaterialParamValueTable;
class MaterialBaseAsset;
struct MaterialParamDesc;

class MaterialBaseImporter : public AssetImporter
{
	
	std::vector<std::byte> LoadShader2Byte(const std::string& RelPath);

	MaterialBaseShaderData AnalShaderData(Json PassJson);
	PipelineBlendMode AnaBlendMode(Json Asset);
	
	std::pair<MaterialParamDescTable, bool> AnaParamDesc(Json Asset);

	void SendDestroy2Swap(size_t MaterialBaseId);
	
public:

	MaterialBaseImporter(const AssetImportData& InData);

	~MaterialBaseImporter() override;

	size_t LoadAsset(Json AssetJson, const std::string& RelPath) override;
	
	void UnloadAsset(AssetObject* Asset) override;
	
};

class MaterialInstanceImporter : public AssetImporter
{

	MaterialParamValueTable AnaParamValue(const MaterialBaseAsset* Base, Json Params);

	void SendDestroy2Swap(size_t MaterialInstanceId);
	
	void ImportScalarParameter(MaterialParamValueTable& Table, const std::string& Name,  const MaterialParamDesc& Desc, Json Param);
	void ImportVectorParameter(MaterialParamValueTable& Table, const std::string& Name,  const MaterialParamDesc& Desc, Json Param);
	void ImportTextureParameter(MaterialParamValueTable& Table, const std::string& Name,  const MaterialParamDesc& Desc, Json Param);

public:
	
	MaterialInstanceImporter(const AssetImportData& InData);

	~MaterialInstanceImporter() override;

	size_t LoadAsset(Json AssetJson, const std::string& RelPath) override;

	void UnloadAsset(AssetObject* Asset) override;
	
};
