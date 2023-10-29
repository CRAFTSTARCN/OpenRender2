#pragma once
#include "RenderSwapDataProcessor.h"
#include "OpenRenderRuntime/Core/RenderPass/RenderBlackboard.h"
#include "OpenRenderRuntime/Core/RenderResource/RenderResource.h"
#include "OpenRenderRuntime/Core/RenderScene/RenderScene.h"
#include "OpenRenderRuntime/Modules/RenderData/MaterialSwapData.h"
#include "OpenRenderRuntime/Modules/RenderData/RenderSwapDataCenter.h"

/*
 * Inefficient implement
 * Should use a more flexible implement
 */
struct RenderDynamicConfig
{
	float ShadowMapLightHeight = 10000.0f;
};

/*
 * Render Class
 * Default render class will provide a basic functional render
 * Leave some functions to virtual for customize
 */
class Render
{

protected:
	
	RenderScene* Scene = nullptr;
	RenderResource* Resources = nullptr;
	RenderBlackboard* BB = nullptr;
	RHI* RenderRHI = nullptr;
	RenderSwapDataProcessor* SwapDataProcessor = nullptr;

	RenderSwapDataCenter* SwapDataCenterPtr = nullptr;

	void SetupGlobalData();

	MeshMaterialPass* MeshDraw = nullptr;
	RenderPass* DeferredShading = nullptr;

	MultiTimeRunable PreThread;
	MultiTimeRunable PostThread;
	
public:

	Render();
	virtual ~Render();

	virtual void Tick(float DeltaTime);
	
	virtual void PreInit();
	virtual void Init(RenderSwapDataCenter* InSwapDataCenter);

	virtual void Terminate();

	virtual void OnAddNewMaterialBase(MaterialBaseCreateData* Data, RenderMaterialBase* MaterialBase);
	virtual void OnMaterialBaseDestroyed(RenderMaterialBase* Destroyed);
	virtual void OnUpdateGlobalIBLResource(const IBLResource& IBL);

	virtual void OnResize(uint32_t Width, uint32_t Height);
	virtual void InitialLoading();

	virtual RenderWindowProxy& GetWindow();
	virtual std::string GetRenderRHIShaderPlatform() const;
	
};
