#pragma once

#include <cstdint>

class RenderScene;
class RenderableInstance;

class RenderQueue 
{

protected:

	RenderScene* Scene = nullptr;
	
public:

	virtual ~RenderQueue();

    virtual void SetScene(RenderScene* InScene);

    virtual void Form() = 0;
	
    virtual void ResetQueue() = 0;
};
