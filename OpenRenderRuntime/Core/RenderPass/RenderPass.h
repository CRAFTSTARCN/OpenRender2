#pragma once
#include <string>
#include <vector>

#include "OpenRenderRuntime/Util/AutoIncreaseIdAllocator.h"
#include "OpenRenderRuntime/Core/RenderPass/RenderBlackboard.h"

class RenderScene;
class RenderResource;
class RHI;

class RenderPass
{

protected:

	inline static AutoIncreaseIdAllocator PassIdAllocator = AutoIncreaseIdAllocator(0);
	
	RenderScene* ScenePtr = nullptr;
	RenderResource* ResourcePtr = nullptr;
	RenderBlackboard* BBPtr = nullptr;
	RHI* RHIPtr = nullptr;

	size_t Id = 0;

	bool Ready = false;

public:

	RenderPass() = default;
	RenderPass(const RenderPass&) = delete;
	RenderPass(RenderPass&&) = delete;
	RenderPass& operator=(const RenderPass&) = delete;
	RenderPass& operator=(RenderPass&&) = delete;
	
	virtual ~RenderPass();

	virtual void PreInit(RenderScene* InScene, RenderResource* InResource, RHI* InRHI, RenderBlackboard* InBlackBoard);

	virtual void Initialize();

	virtual void PostInit();
	
	virtual void Terminate() = 0;
	
	virtual void DrawPass() = 0;

	virtual void OnResize(uint32_t Width, uint32_t Height) = 0;

	bool GetPassReadyState() const;

	virtual std::string GetRenderPassName() const = 0;

	virtual size_t GetRenderPassId() const;

	virtual std::vector<std::byte> LoadPassShaderCode(const std::string RelPath);
};