#pragma once
#include <unordered_map>

#include "RenderTexture.h"
#include "OpenRenderRuntime/Core/RenderResource/RenderMesh.h"
#include "OpenRenderRuntime/Core/RenderResource/RenderMaterialInstance.h"
#include "OpenRenderRuntime/Core/RenderResource/RenderMaterialBase.h"
#include "OpenRenderRuntime/Core/RHI/RHI.h"
#include "OpenRenderRuntime/Core/RHI/RHITexture.h"

struct GBuffer
{
	RHITexture* GBufferA = nullptr; //RGB: base color, A: baked ambient occlusion
	RHITexture* GBufferB = nullptr; //RGB: Normal, A: subsurface
	RHITexture* GBufferC = nullptr; //RGB: metallic, specular, roughness, A: shader model id
	RHITexture* GBufferD = nullptr; //RGB: emissive
	RHITexture* GBufferE = nullptr; //RGB: Tangent A: Anisotropy
};

//Gpu data proxy
struct GlobalRenderDataProxy
{
	float camera_position[3];
	float camera_position_padding;
	
	float directional_light_direction[3];
	float directional_light_direction_padding;
	
	float directional_light_color[3];
	float directional_light_color_padding;
	
	float ambient_light_color[3];
	float ambient_light_color_padding;
	
	float camera_frustum[4][6];
    
	float mat_view_proj[4][4];
	float mat_view_proj_inv[4][4];
	float directional_light_view_proj[4][4];
};

struct IBLResource
{
	RenderTexture* SkyBox = nullptr;
	RenderTexture* RadianceMap = nullptr;
	RenderTexture* IrradianceMap = nullptr;
	RenderTexture* BRDFLUT = nullptr;
};

struct DefaultErrorResource
{
	RenderTexture* DefaultTexture = nullptr;
	RenderMaterialInstance* DefaultMaterialInstance = nullptr;
};


class RenderResource
{

	RHI* RHIPtr = nullptr;
	size_t CurrentOffset = 0;
	
public:
	

	virtual ~RenderResource() {}
	
	std::unordered_map<size_t, RenderTexture*> Textures {};
	std::unordered_map<size_t, RenderMaterialBase*> MaterialBases {};
	std::unordered_map<size_t, RenderMaterialInstance*> Materials {};
	std::unordered_map<size_t, RenderMesh*> Meshes;

	/*
	 * Resources registered by render passes that may used by other passes
	 * The resource should destroyed by the pass that created it
	 */
	std::unordered_map<std::string, RHITexture*> RegisteredTextures;
	std::unordered_map<std::string, RHIDescriptorLayout*> RegisteredLayout;
	std::unordered_map<std::string, RHIDescriptorSet*> RegisteredSet;
	std::unordered_map<std::string, RHISampler*> RegisteredSampler;
	
	GBuffer GlobalGBuffer {};
	RHITexture* DepthStencilAttachment = nullptr;

	RHIBuffer* GlobalBuffer = nullptr;
	size_t GlobalBufferSize = 0;
	size_t GlobalBufferMinAlignment = 16;
	std::mutex GlobalBufferMutex;

	RHIBuffer* GlobalRenderDataBuffer = nullptr;
	IBLResource GlobalIBLResource {};
	DefaultErrorResource DefaultResources {};
	
	RHIDescriptorLayout* MeshLayout = nullptr;
	
	virtual void Initialize(RHI* InRHI);

	size_t GetGlobalBufferCurrentOffset();
	void AddGlobalBufferOffset(size_t Add);

	virtual void SetGlobalDataBuffer(const GlobalRenderDataProxy& Data);

	void Prepare();

	virtual void ClearResource();

	virtual void Terminate();

	void OnCreateGlobalRenderDataBuffer();
		
	/*
	 * Make these virtual for better pipeline customize
	 */

	virtual void OnCreateDepthBuffer(uint32_t Width, uint32_t Height);
	
	virtual void OnCreateGBuffer(uint32_t Width, uint32_t Height);

	virtual void OnResize(uint32_t Width, uint32_t Height);

	virtual void OnCreateMeshLayout();

	/*
	 * Resource destroy function
	 */

	virtual void DestroyMeshResource(RenderMesh*& Mesh);

	virtual void DestroyMaterialBaseResource(RenderMaterialBase* MaterialBase);

	virtual void DestroyMaterialInstanceResource(RenderMaterialInstance* MaterialInstance);
};
