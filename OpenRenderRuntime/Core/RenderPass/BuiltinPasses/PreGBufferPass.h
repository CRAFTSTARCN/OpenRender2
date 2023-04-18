#pragma once
#include "OpenRenderRuntime/Core/RenderPass/MeshMaterialPass.h"
#include "OpenRenderRuntime/Core/RenderScene/Queues/DefaultMaterialBasedQueue.h"
#include "OpenRenderRuntime/Core/RHI/RHIFrameBuffer.h"
#include "OpenRenderRuntime/Core/RHI/RHIRenderPass.h"
#include "OpenRenderRuntime/Core/RHI/RHISemaphore.h"
#include "OpenRenderRuntime/Core/RHI/RHIShaderModule.h"

class  PreGBufferPass : public MeshMaterialPass
{
	RHIRenderPass* Pass = nullptr;
	RHIFrameBuffer* FrameBuffer = nullptr;
	RHISemaphore* SignaledSemaphore = nullptr;

	RHIDescriptorLayout* GlobalDataLayout = nullptr;
	RHIDescriptorSet *GlobalDataSet = nullptr;
	RHIShaderModule* TaskShader = nullptr;

	RHIDescriptorLayout* DrawCallLayout = nullptr;
	RHIDescriptorSet* DrawCallSet = nullptr;

	Semaphore* SubmitSemaphore = nullptr;

	DefaultMaterialBasedQueue Queue;

	
	void CreateRenderPass();

	void CreateFrameBuffer(uint32_t Width, uint32_t Height);

	void CreateGlobalDataSetAndLayout();

	void CreateDrawCallDataSetAndLayout();

	void LoadPassTaskShader();

	void CreateSemaphores();
	
public:

	PreGBufferPass();
	~PreGBufferPass() override;

	void OnResize(uint32_t Width, uint32_t Height) override;

	void Initialize() override;

	void DrawPass() override;
	
	void Terminate() override;
	
	void OnCreateMaterialBase(MaterialBaseCreateData* Data, RenderMaterialBase* NewMaterialBase) override;
	void OnDestroyMaterialBase(RenderMaterialBase* DestroyedMaterialBase) override;
	
	std::string GetRenderPassName() const override;
};
