#include "OpenRenderRuntime/Core/RenderPass/RenderPass.h"

#include <filesystem>
#include <fstream>

#include "OpenRenderRuntime/Core/RHI/RHI.h"
#include "OpenRenderRuntime/Modules/RenderConfig/RenderCofig.h"
#include "OpenRenderRuntime/Util/FileUtil.h"
#include "OpenRenderRuntime/Util/Logger.h"

RenderPass::~RenderPass() = default;

void RenderPass::PreInit(RenderScene* InScene, RenderResource* InResource, RHI* InRHI, RenderBlackboard* InBlackBoard)
{
	ScenePtr = InScene;
	ResourcePtr = InResource;
	RHIPtr = InRHI;
	BBPtr = InBlackBoard;
}

void RenderPass::Initialize()
{
	Id = PassIdAllocator.GetNewId();
}

void RenderPass::PostInit()
{
	Ready = true;
}

void RenderPass::Terminate()
{
}

bool RenderPass::GetPassReadyState() const
{
	return  Ready;
}

size_t RenderPass::GetRenderPassId() const
{
	return Id;
}

std::vector<std::byte> RenderPass::LoadPassShaderCode(const std::string RelPath)
{
	std::filesystem::path Path = RenderConfig::Get().RootDir / std::filesystem::path(RenderConfig::Get().PipelineShaderDir) / RHIPtr->GetShaderNameSuffix()/ RelPath;
	std::string FullPath = Path.generic_string() + std::string(".") + RHIPtr->GetShaderNameSuffix();

	std::vector<std::byte> Bytes = FileUtil::LoadFile2Byte(FullPath);
	if(Bytes.empty())
	{
		LOG_ERROR_FUNCTION("Cannot open shader file for pass or empty, given path: {0}", RelPath);
	}
	
	return Bytes;
}
