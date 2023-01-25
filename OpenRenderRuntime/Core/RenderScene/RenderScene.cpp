#include "RenderScene.h"

#include <queue>

#include "OpenRenderRuntime/Core/Basic/RenderMath.h"

void RenderScene::FrustumCullingFunction(const Frustum& F, size_t Left, size_t Right, int Bit)
{
    for(size_t i = Left; i < Right && i < Instances.size(); ++i)
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
    CullingThreadCount = std::thread::hardware_concurrency() >> 1;
    CullingThreads = new MultiTimeRunable[CullingThreadCount];
}

void RenderScene::Terminate()
{
    Instances.clear();
    RenderableIdIndexTable.clear();
    delete[] CullingThreads;
}

void RenderScene::SetupCameraVisibility()
{
    Frustum& F = Camera.GetFrustum();
    if(Instances.size() <= 1)
    {
        FrustumCullingFunction(F, 0, Instances.size(), 0);
    } else
    {
        if(Instances.size() < CullingThreadCount)
        {
            for(size_t i=0; i<Instances.size(); ++i)
            {
                CullingThreads[i].Run(
                    &RenderScene::FrustumCullingFunction,
                    this,
                    F,
                    i,
                    i+1,
                    0);
            }

            for(size_t i=0; i<Instances.size(); ++i)
            {
                CullingThreads[i].WaitForPushable();
            }
        }
        else
        {
            size_t Total = Instances.size();
            size_t Base = Total / CullingThreadCount;
            size_t Bonus = Total % CullingThreadCount;

            size_t Left = 0;
            for(size_t i = 0; i < CullingThreadCount; ++i)
            {
                size_t Right = Left + Base;
                if(i < Bonus)
                {
                    ++Right;
                }

                CullingThreads[i].Run(
                    &RenderScene::FrustumCullingFunction,
                    this,
                    F,
                    Left,
                    Right,
                    0);
            }

            for(size_t i = 0; i < CullingThreadCount; ++i)
            {
                CullingThreads[i].WaitForPushable();
            }
        }
    }

    
}

void RenderScene::FormQueues()
{
    size_t ThisTime = 0;
    auto InvalidInstance = [](RenderableInstance& Instance) -> bool
    {
        return !Instance.IsValid ||
               !(Instance.Visible & 1) ||
               Instance.MaterialPtr->Base->BlendMode == PipelineBlendMode_Translucent ||
               Instance.MaterialPtr->Base->BlendMode == PipelineBlendMode_Additive;
    };
    
   if(TotalBlocker == 0)
   {
       //Half of max pre rendered
       uint32_t PreRendered = std::min(MaxPreRendered >> 1, TotalInstance);
       size_t CurrentIndex = 0, UsedInstance = 0;
       for(;CurrentIndex < Instances.size() && UsedInstance < PreRendered; ++CurrentIndex)
       {
           RenderableInstance& Instance = Instances[CurrentIndex];
           if(InvalidInstance(Instance))
           {
               continue;
           }

           ++ThisTime;
           PreRenderedOpaquedQueue.InsertInstance(Instance, CurrentIndex);
           ++UsedInstance;
       }

       while(CurrentIndex < Instances.size())
       {
           RenderableInstance& Instance = Instances[CurrentIndex];
           if(InvalidInstance(Instance))
           {
               continue;
           }

           ++ThisTime;
           PostRenderedOpaquedQueue.InsertInstance(Instance, CurrentIndex);
           ++CurrentIndex;
       }
   }
   else
   {
       if(TotalBlocker <= MaxPreRendered)
       {
           /*
            * With blocker in scene but less than max
            * Use all blocker
            */
           for(size_t i = 0; i<Instances.size(); ++i)
           {
               RenderableInstance& Instance = Instances[i];
               if(InvalidInstance(Instance))
               {
                   continue;
               }

               if(Instances[i].IsBlocker)
               {
                   ++ThisTime;
                   PreRenderedOpaquedQueue.InsertInstance(Instance, i);
               }
               else
               {
                   ++ThisTime;
                   PostRenderedOpaquedQueue.InsertInstance(Instance, i);
               }
           }
       }
       else
       {
           /*
            * With blocker in scene and more than needed
            * Render MaxPreRendered at first pass for occlusion culling
            */
           std::priority_queue<std::pair<float, size_t>> DistanceQueue;
           for(size_t i = 0; i<Instances.size(); ++i)
           {
               RenderableInstance& Instance = Instances[i];
               if(InvalidInstance(Instance))
               {
                   continue;
               }

               if(Instances[i].IsBlocker)
               {
                   //Forth vector of model matrix represent the translate of instance
                   float Dist = glm::distance(Camera.GetPosition(), glm::vec3(Instance.CachedModelMatrix[3]));
                   DistanceQueue.emplace(Dist, i);
               }
               else
               {
                   ++ThisTime;
                   PostRenderedOpaquedQueue.InsertInstance(Instance, i);
               }

               size_t UsedBlocker = 0;
               while(!DistanceQueue.empty() && UsedBlocker < MaxPreRendered)
               {
                   auto &[Distance, Index] = DistanceQueue.top();
                   ++ThisTime;
                   PreRenderedOpaquedQueue.InsertInstance(Instances[Index], Index);
                   DistanceQueue.pop();
                   ++UsedBlocker;
               }

               while(!DistanceQueue.empty())
               {
                   auto &[Distance, Index] = DistanceQueue.top();
                   ++ThisTime;
                   PostRenderedOpaquedQueue.InsertInstance(Instances[Index], Index);
                   DistanceQueue.pop();
               }
           }
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
    PostRenderedOpaquedQueue.ResetQueue();
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
    if(Instance.IsBlocker)
    {
        TotalBlocker++;
    }
    TotalInstance++;
}
