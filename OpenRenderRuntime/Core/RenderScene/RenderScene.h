#pragma once
#include <vector>

#include "RenderQueue.h"
#include "OpenRenderRuntime/Core/RenderScene/Lights.h"
#include "OpenRenderRuntime/Core/RenderScene/RenderableInstance.h"
#include "OpenRenderRuntime/Core/RenderScene/SceneCamera.h"
#include "OpenRenderRuntime/Util/MultiTimeRunable.hpp"
#include "OpenRenderRuntime/Core/RenderScene/Queues/DefaultMaterialBasedQueue.h"

class RenderScene
{

protected:
    
    std::unordered_map<std::size_t, size_t> RenderableIdIndexTable;
    
    MultiTimeRunable* SceneThreads = nullptr;
    size_t SceneThreadCount = 0;

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
	std::unordered_map<std::string, RenderQueue*> Queues;
	
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

	virtual void RegisterQueue(const std::string& Name, RenderQueue* RegisteredQueue);
	
	template<typename FT, typename... ArgT>
	void RunSceneTask(int ThreadIndex, FT&& Func, ArgT&& ...Args)
	{
		if(ThreadIndex <= SceneThreadCount || ThreadIndex < 0)
		{
			LOG_ERROR_FUNCTION("{0} out of scene thread index", ThreadIndex);
			return;
		}

		SceneThreads[ThreadIndex].Run(std::forward<FT&&>(Func), std::forward<ArgT&&>(Args)...);
	}
};
