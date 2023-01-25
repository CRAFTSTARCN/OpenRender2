#pragma once
#include "OpenRenderRuntime/Modules/RenderData/RenderSwapData.h"
#include "OpenRenderRuntime/Core/RenderPass/MeshMaterialPass.h"

#define DYNAMIC_CAST_DATA_CHECK(DataType, Var, NewName) \
	DataType* NewName = dynamic_cast<DataType*>(Var);\
	if(!NewName) {\
		LOG_ERROR_FUNCTION("Invalid cast swap data type: {0}", #DataType);\
		delete (Var);\
		return;\
	}

class Render;

struct RenderComponentsData
{
	Render* RenderPtr = nullptr;
	RenderResource* ResourcePtr = nullptr;
	RenderScene* ScenePtr = nullptr;
	RHI* RHIPtr = nullptr;
};

class RenderSwapDataResolver
{

protected:
	
	Render* RenderPtr = nullptr;
	RenderResource* ResourcePtr = nullptr;
	RenderScene* ScenePtr = nullptr;
	RHI* RHIPtr = nullptr;

public:

	RenderSwapDataResolver(const RenderComponentsData& InData)
	{
		RenderPtr = InData.RenderPtr;
		ResourcePtr = InData.ResourcePtr;
		ScenePtr = InData.ScenePtr;
		RHIPtr = InData.RHIPtr;
	}
	
	virtual ~RenderSwapDataResolver() = default;
	virtual void ResolveData(RenderSwapData* Data) = 0;
};
