#pragma once
#include <vector>

#include "OpenRenderRuntime/Core/RenderScene/Lights.h"
#include "OpenRenderRuntime/Core/RenderScene/RenderableInstance.h"
#include "OpenRenderRuntime/Core/RenderScene/SceneCamera.h"
#include "OpenRenderRuntime/Util/MultiTimeRunable.hpp"
#include "OpenRenderRuntime/Core/RenderScene/Queues/DefaultMaterialBasedQueue.h"

class RenderScene
{

protected:
    
    std::unordered_map<std::size_t, size_t> RenderableIdIndexTable;
    
    MultiTimeRunable* CullingThreads = nullptr;
    size_t CullingThreadCount = 0;

    constexpr static float MaxEmptyLoad = 0.33f;
    constexpr static int MaxEmptyNum = 128;
    constexpr static uint32_t MaxPreRendered = 128;

    uint32_t TotalInstance = 0;
    uint32_t TotalBlocker = 0;
    
    void FrustumCullingFunction(const Frustum& F, size_t Left, size_t Right, int Bit);

	void ArrangeInstances();
    
public:

    SceneCamera Camera {};
    DirectionalLight Light {};
	glm::vec3 AmbientLight {};
    
    std::vector<RenderableInstance> Instances;
    
    DefaultMaterialBasedQueue PreRenderedOpaquedQueue {};
    DefaultMaterialBasedQueue PostRenderedOpaquedQueue {};
    
    virtual ~RenderScene();
    
    virtual void Initialize();

    virtual void Terminate();

    virtual void SetupCameraVisibility();

    virtual void FormQueues();

	virtual void Prepare();

	/*
	 * If empty load of instances over max, re-arrange instances in OnPostTick function
	 */
    virtual void OnPostTick();

    //Tool functions
    virtual bool HasInstance(size_t Id);

    /*
     * Direct means the instance will add to scene directly, no existence check
     * If instance exist, new added instance will ignored
     * HasInstance function will check if a instance in scene
     */
    virtual void TryAddInstance(const RenderableInstance& Instance);
    
};
