#pragma once
#include "OpenRenderRuntime/Core/RenderPass/MeshMaterialPass.h"
#include "OpenRenderRuntime/Core/RenderPass/PassInterface/IIBLPass.h"
#include "OpenRenderRuntime/Core/RHI/RHIFrameBuffer.h"
#include "OpenRenderRuntime/Core/RHI/RHIRenderPass.h"
#include "OpenRenderRuntime/Core/RHI/RHISemaphoreView.h"

/*
 * DeferredShading, shading 
 */
class DeferredShadingPass : public RenderPass, public IIBLPass
{
	
	RHIRenderPass* Pass = nullptr;
	
	RHIPipeline* DeferredShadingPipeline = nullptr;
	std::vector<RHIFrameBuffer*> FrameBuffers {};
	
	RHIDescriptorLayout* DeferredShadingGlobalLayout = nullptr;
	RHIDescriptorLayout* DeferredShadingGBufferLayout = nullptr;
	RHIDescriptorSet* DeferredShadingGlobal = nullptr;
	RHIDescriptorSet* DeferredShadingGBuffer = nullptr;

	RHISemaphoreView* WaitView = nullptr;
	Semaphore* SubmitSemaphore = nullptr;
	
	void CreateRenderPass();
	
	void CreateDeferredShadingDescriptors();
	
	void UpdateGBufferAndDepth();

	void UpdateGlobalTexture(const IBLResource& IBL);

	void CreateFrameBuffers(uint32_t Width, uint32_t Height);

	void CreatePipeline();

	void CreateSyncView();
	
public:
	
	void Initialize() override;

	void DrawPass() override;

	void Terminate() override;

	void OnResize(uint32_t Width, uint32_t Height) override;

	void OnUpdateIBLResource(const IBLResource& IBL) override;

	std::string GetRenderPassName() const override;
};
