#include "RenderSwapDataProcessor.h"

#include "OpenRenderRuntime/Modules/RenderData/RenderableInstanceData.h"
#include "OpenRenderRuntime/Modules/RenderData/SceneSwapData.h"
#include "OpenRenderRuntime/Modules/RenderData/Resolvers/MaterialDataResolvers.h"
#include "OpenRenderRuntime/Modules/RenderData/Resolvers/MeshDataResolvers.h"
#include "OpenRenderRuntime/Modules/RenderData/Resolvers/RenderableInstanceDataResolvers.h"
#include "OpenRenderRuntime/Modules/RenderData/Resolvers/SceneDataResolvers.h"
#include "OpenRenderRuntime/Modules/RenderData/Resolvers/TextureDataResolvers.h"
#include "OpenRenderRuntime/Util/Logger.h"

RenderSwapDataProcessor::~RenderSwapDataProcessor()
{
}

void RenderSwapDataProcessor::Initialize(const RenderComponentsData& InData)
{
	Resolvers = {
		{STR_CLASS(TextureCreateData), new TextureCreateDataResolver(InData)},
		{STR_CLASS(MeshCreateData), new MeshDataCreateResolver(InData)},
		{STR_CLASS(MeshDestroyData), new MeshDataDestroyResolver(InData)},
		{STR_CLASS(MaterialBaseCreateData), new MaterialBaseDataCreateResolver(InData)},
		{STR_CLASS(MaterialInstanceCreateData), new MaterialInstanceDataCreateResolver(InData)},
		{STR_CLASS(RenderableInstanceAdd), new RenderableInstanceAddDataResolver{InData}},
		{STR_CLASS(EnvironmentTextureData),new EnvironmentDataResolver(InData)},
		{STR_CLASS(CameraSetData), new CameraSetResolver(InData)},
		{STR_CLASS(CameraPositionLookData), new CameraPositionLookResolver{InData}},
		{STR_CLASS(DirectionalLightData), new DirectionalLightResolver(InData)}
	};
	ResolverCache.resize(Resolvers.size(), nullptr);
}

void RenderSwapDataProcessor::ResolveData(RenderSwapData* Data)
{
	if(ResolverCache[Data->GetTypeId()] != nullptr)
	{
		ResolverCache[Data->GetTypeId()]->ResolveData(Data);
	}
	else
	{
		if(auto Iter = Resolvers.find(Data->GetDataType()); Iter == Resolvers.end())
		{
			LOG_ERROR_FUNCTION("Fail to find resolver for {}", Data->GetDataType());
			delete Data;
		}
		else
		{
			ResolverCache[Data->GetTypeId()] = Iter->second;
			Iter->second->ResolveData(Data);
		}
	}
	
}

void RenderSwapDataProcessor::ProcessSwapData(const std::vector<RenderSwapData*>& SwapData)
{
	for(auto Data : SwapData)
	{
		ResolveData(Data);
	}
}

void RenderSwapDataProcessor::Terminate()
{
	for(auto &[Tp, Resolver] : Resolvers)
	{
		delete Resolver;
	}

	Resolvers.clear();
}
