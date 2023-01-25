#include "Render.h"

#include <glm/gtc/type_ptr.hpp>

#include "OpenRenderRuntime/Core/Render/RenderSwapDataProcessor.h"
#include "OpenRenderRuntime/Core/RenderPass/BuiltinPasses/PostCombinedPass.h"
#include "OpenRenderRuntime/Core/RenderPass/BuiltinPasses/PreGBufferPass.h"
#include "OpenRenderRuntime/Core/VulkanRHI/VulkanRHI.h"

void Render:: SetupGlobalData()
{
	GlobalRenderDataProxy GlobalData {};
	memcpy(GlobalData.camera_position, glm::value_ptr(Scene->Camera.GetPosition()), 3*sizeof(float));
	memcpy(GlobalData.directional_light_direction, glm::value_ptr(Scene->Light.DirectionL), 3*sizeof(float));
	memcpy(GlobalData.directional_light_color, glm::value_ptr(Scene->Light.Color), 3*sizeof(float));
	memcpy(GlobalData.ambient_light_color, glm::value_ptr(Scene->AmbientLight), 3*sizeof(float));
	memcpy(GlobalData.camera_frustum, (void*)&Scene->Camera.GetFrustum(), 4ull * 6 * sizeof(float));
	memcpy(GlobalData.mat_view_proj, glm::value_ptr(Scene->Camera.GetVPMatrix()), 16*sizeof(float));

	glm::mat4 VPInv = glm::inverse(Scene->Camera.GetVPMatrix());

	memcpy(GlobalData.mat_view_proj_inv, glm::value_ptr(VPInv), 16 * sizeof(float));
	Resources->SetGlobalDataBuffer(GlobalData);
}

Render::Render()
{
	RenderRHI = new VulkanRHI;
	Resources = new RenderResource;
	BB = new RenderBlackboard;
	Scene = new RenderScene;

	SwapDataProcessor = new RenderSwapDataProcessor;

	//Allocate render pass

	PreGBuffer = new PreGBufferPass;
	Post = new PostCombinedPass;
}

Render::~Render()
{
	delete Scene;
	delete BB;
	delete Resources;
	delete RenderRHI;
}

void Render::Tick(float DeltaTime)
{
	SwapDataProcessor->ProcessSwapData(SwapDataCenterPtr->GetRenderSide());
	Resources->Prepare();
	Scene->Prepare();

	Scene->SetupCameraVisibility();
	Scene->FormQueues();
	
	SetupGlobalData();

	RenderRHI->BeginFrameRendering();
	PreGBuffer->DrawPass();
	Post->DrawPass();
	RenderRHI->EndFrameRendering();

	Scene->OnPostTick();
	
}

void Render::PreInit()
{
	
	RenderRHI->PreInit();
}

void Render::Init(RenderSwapDataCenter* InSwapDataCenter)
{

	PreGBuffer->PreInit(Scene, Resources, RenderRHI, BB);
	Post->PreInit(Scene, Resources, RenderRHI, BB);
	
	SwapDataCenterPtr = InSwapDataCenter;
	SwapDataProcessor->Initialize(RenderComponentsData{this, Resources, Scene, RenderRHI});
	RenderRHI->Init();
	Resources->Initialize(RenderRHI);
	Scene->Initialize();

	PreGBuffer->Initialize();
	Post->Initialize();

	PreGBuffer->PostInit();
	Post->PostInit();
	/*
 	 * Initial loading process
 	 */
	RenderRHI->SetResizeFunction([this](uint32_t Width, uint32_t Height)
	{
		this->OnResize(Width, Height);
	});
}

void Render::Terminate()
{
	RenderRHI->PreTerminate();

	Post->Terminate();
	PreGBuffer->Terminate();
	Scene->Terminate();
	
	Resources->Terminate();
	RenderRHI->Terminate();
}

void Render::OnAddNewMaterialBase(MaterialBaseCreateData* Data, RenderMaterialBase* MaterialBase)
{
	PreGBuffer->OnCreateMaterialBase(Data, MaterialBase);
}

void Render::OnMaterialBaseDestroyed(RenderMaterialBase* Destroyed)
{
	PreGBuffer->OnDestroyMaterialBase(Destroyed);
}

void Render::OnUpdateGlobalIBLResource(const IBLResource& IBL)
{
	if(Post->GetPassReadyState())
	{
		IIBLPass* PostCombinedPtr = dynamic_cast<IIBLPass*>(Post);
		PostCombinedPtr->OnUpdateIBLResource(IBL);
	}
}

void Render::OnResize(uint32_t Width, uint32_t Height)
{
	Resources->OnResize(Width, Height);
	PreGBuffer->OnResize(Width, Height);
	Post->OnResize(Width, Height);
}

void Render::InitialLoading()
{
	SwapDataProcessor->ProcessSwapData(SwapDataCenterPtr->GetRenderSide());
}

RenderWindowProxy& Render::GetWindow()
{
	return RenderRHI->GetWindowProxy();
}

std::string Render::GetRenderRHIShaderPlatform() const
{
	return RenderRHI->GetShaderNameSuffix();
}
