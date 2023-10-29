#include "Render.h"

#include <glm/gtc/type_ptr.hpp>

#include "OpenRenderRuntime/Core/Render/RenderSwapDataProcessor.h"
#include "OpenRenderRuntime/Core/RenderPass/BuiltinPasses/DeferredShadingPass.h"
#include "..\RenderPass\BuiltinPasses\MeshDrawPass.h"
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

	MeshDraw = new MeshDrawPass;
	DeferredShading = new DeferredShadingPass;
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
	PreThread.Run([&](){MeshDraw->DrawPass();});
	PostThread.Run([&](){DeferredShading->DrawPass();});

	PreThread.WaitForPushable();
	PostThread.WaitForPushable();
	RenderRHI->EndFrameRendering();

	Scene->OnPostTick();
	
}

void Render::PreInit()
{
	
	RenderRHI->PreInit();
}

void Render::Init(RenderSwapDataCenter* InSwapDataCenter)
{

	MeshDraw->PreInit(Scene, Resources, RenderRHI, BB);
	DeferredShading->PreInit(Scene, Resources, RenderRHI, BB);
	
	SwapDataCenterPtr = InSwapDataCenter;
	SwapDataProcessor->Initialize(RenderComponentsData{this, Resources, Scene, RenderRHI});
	RenderRHI->Init();
	Resources->Initialize(RenderRHI);
	Scene->Initialize();

	MeshDraw->Initialize();
	DeferredShading->Initialize();

	MeshDraw->PostInit();
	DeferredShading->PostInit();
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
	
	DeferredShading->Terminate();
	MeshDraw->Terminate();

	PreThread.WaitForPushable();
	PostThread.WaitForPushable();
	
	Scene->Terminate();
	
	Resources->Terminate();
	RenderRHI->Terminate();
}

void Render::OnAddNewMaterialBase(MaterialBaseCreateData* Data, RenderMaterialBase* MaterialBase)
{
	MeshDraw->OnCreateMaterialBase(Data, MaterialBase);
}

void Render::OnMaterialBaseDestroyed(RenderMaterialBase* Destroyed)
{
	MeshDraw->OnDestroyMaterialBase(Destroyed);
}

void Render::OnUpdateGlobalIBLResource(const IBLResource& IBL)
{
	if(DeferredShading->GetPassReadyState())
	{
		IIBLPass* PostCombinedPtr = dynamic_cast<IIBLPass*>(DeferredShading);
		PostCombinedPtr->OnUpdateIBLResource(IBL);
	}
}

void Render::OnResize(uint32_t Width, uint32_t Height)
{
	Resources->OnResize(Width, Height);
	MeshDraw->OnResize(Width, Height);
	DeferredShading->OnResize(Width, Height);
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
