#include "OpenRenderRuntime/Core/RenderScene/RenderQueue.h"

RenderQueue::~RenderQueue()
{
}

void RenderQueue::SetScene(RenderScene* InScene)
{
    Scene = InScene;
}
