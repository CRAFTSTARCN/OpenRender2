#include "RenderScene.h"

#include <queue>

#include "OpenRenderRuntime/Core/Basic/RenderMath.h"

void RenderScene::FrustumCullingFunction(const Frustum& F, size_t Left, size_t Right, int Bit)
{
    for(size_t i = Left; i < Right ; ++i)
    {
        RenderableInstance& Instance = Instances[i];
        if(!Instance.IsValid)
        {
            continue;
        }
        int Intersect = ORMath::AABBIntersectsFrustum(Instance.CachedAABB, F);
        Instance.Visible |= (Intersect << Bit);
    }

}

void RenderScene::ArrangeInstances()
{
    std::vector<RenderableInstance> NewInstances(TotalInstance);
    RenderableIdIndexTable.clear();

    size_t NewIndex = 0;
    for(size_t i=0; i<Instances.size(); ++i)
    {
        if(!Instances[i].IsValid)
        {
            continue;
        }

        NewInstances[NewIndex] = Instances[i];
        RenderableIdIndexTable[Instances[i].InstanceID] = NewIndex;
        NewIndex++;
    }

    Instances.swap(NewInstances);
}

RenderScene::~RenderScene() = default;

void RenderScene::Initialize()
{
    SceneThreadCount = std::thread::hardware_concurrency() >> 1;
    if(SceneThreadCount != 0)
    {
        SceneThreads = new MultiTimeRunable[SceneThreadCount];
    }
}

void RenderScene::Terminate()
{
    Instances.clear();
    RenderableIdIndexTable.clear();
    delete[] SceneThreads;
}

void RenderScene::SetupCameraVisibility()
{
    Frustum& F = Camera.GetFrustum();
    if(Instances.size() < SceneThreadCount || SceneThreadCount == 0)
    {
        FrustumCullingFunction(F, 0, Instances.size(), 0);
    } else
    {
        size_t Total = Instances.size();
        size_t Base = Total / SceneThreadCount;

        size_t Left = 0;
        for(size_t i = 0; i < SceneThreadCount - 1; ++i)
        {
            size_t Right = Left + Base;
            
            SceneThreads[i].Run(
                &RenderScene::FrustumCullingFunction,
                this,
                F,
                Left,
                Right,
                0);
            
            Left = Right;
        }

        SceneThreads[SceneThreadCount - 1].Run(
                &RenderScene::FrustumCullingFunction,
                this,
                F,
                Left,
                Total,
                0);

        for(size_t i = 0; i < SceneThreadCount; ++i)
        {
            SceneThreads[i].WaitForPushable();
        }
    }
}

void RenderScene::FormQueues()
{
    size_t ThisTime = 0;
    
    for(auto & Instance : Instances)
    {
        if(Instance.IsValid &&
           (Instance.Visible & 1) &&
           (Instance.MaterialPtr->Base->BlendMode == PipelineBlendMode_Opaque || Instance.MaterialPtr->Base->BlendMode == PipelineBlendMode_Masked))
        {
            ++ThisTime;
            PreRenderedOpaquedQueue.InsertInstance(Instance, Instance.InstanceID);
        }
    }
    
    static size_t LastTime = 0;

    if(ThisTime != LastTime)
    {
        LOG_INFO_FUNCTION("Current rendering instance count :{0}", ThisTime);
        LastTime = ThisTime;
    }
}

void RenderScene::Prepare()
{
    PreRenderedOpaquedQueue.ResetQueue();
}

void RenderScene::OnPostTick()
{
    uint32_t IndexCount = (uint32_t)Instances.size();
    uint32_t Diff = IndexCount - TotalInstance;
    if((float)Diff / (float)IndexCount >= MaxEmptyLoad || Diff >= MaxEmptyNum)
    {
        ArrangeInstances();
    }

    for(auto& Instance : Instances)
    {
        Instance.Visible = 0;
    }
}

bool RenderScene::HasInstance(size_t Id)
{
    return RenderableIdIndexTable.find(Id) != RenderableIdIndexTable.end();
}

void RenderScene::TryAddInstance(const RenderableInstance& Instance)
{
    auto [Iter, Result] = RenderableIdIndexTable.try_emplace(Instance.InstanceID, Instances.size());
    if(!Result)
    {
        LOG_ERROR_FUNCTION("Duplicate instance id");
        return;
    }
    
    
    Instances.push_back(Instance);
    TotalInstance++;
}
