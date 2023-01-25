#pragma once
#include "OpenRenderRuntime/Core/RenderPass/RenderPass.h"
#include "OpenRenderRuntime/Core/RenderResource/RenderMaterialBase.h"
#include "OpenRenderRuntime/Modules/RenderData/MaterialSwapData.h"
#include "OpenRenderRuntime/Util/RecycleIdAllocator.h"

/*
 * Material pass is a render pass that will use material data of renderable
 * For example: a gbuffer pass
 */
class MeshMaterialPass : public RenderPass
{

protected:
	
	inline static RecycleIdAllocator MatIdAllocator = RecycleIdAllocator(0);
	size_t MaterialPassId = 0;
	
public:

	virtual void Initialize() override;

	virtual void Terminate() override;
	
	virtual size_t GetMaterialPassId();

	virtual void OnCreateMaterialBase(MaterialBaseCreateData* Data, RenderMaterialBase* NewMaterialBase) = 0;

	virtual void OnDestroyMaterialBase(RenderMaterialBase* DestroyedMaterialBase) = 0;
};