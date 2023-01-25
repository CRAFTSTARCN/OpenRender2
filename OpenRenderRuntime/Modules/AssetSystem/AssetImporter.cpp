#include "OpenRenderRuntime/Modules/AssetSystem/AssetImporter.h"



AssetImporter::AssetImporter(const AssetImportData& InData) :
	RegistryPtr(InData.RegistryPtr), SwapDataCenterPtr(InData.SwapDataCenterPtr), ConfigPtr(InData.ConfigPtr)
{
}

void AssetImporter::SetCascadeLoadingFunction(const std::function<size_t(const std::string&)>& Func)
{
	OnCascadeLoading = Func;
}

AssetImporter::~AssetImporter()
{
}

ParamUsage AssetImporter::AnaParamUsage(const std::vector<Json>& UsageJson)
{
	ParamUsage Usage = 0;
	for(const auto& UsageStr : UsageJson)
	{
		if(UsageStr.string_value() == "Frag")
		{
			Usage |= ParamUsageBit_Fragment;
		}
		else if(UsageStr.string_value() == "Geo")
		{
			Usage |= ParamUsageBit_Geometry;	
		}
	}
	return Usage == 0 ? ParamUsageBit_Fragment | ParamUsageBit_Geometry : Usage;
}
