#include "OpenRenderRuntime/Core/RenderPass/RenderPass.h"

#include <filesystem>
#include <fstream>

#include "OpenRenderRuntime/Core/RHI/RHI.h"
#include "OpenRenderRuntime/Modules/RenderConfig/RenderCofig.h"
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
	std::ifstream In;
	std::vector<std::byte> Bytes;
	std::string FullPath = Path.generic_string() + std::string(".") + RHIPtr->GetShaderNameSuffix();
	In.open(FullPath, std::ios::in|std::ios::binary);

	if(!In.is_open())
	{
		LOG_ERROR_FUNCTION("Cannot open shader file for pass, given path: {0}", RelPath);
		return {};
	}
	
	In.seekg(0, std::ios::end);
	std::streamsize Len = In.tellg();
	In.seekg(0, std::ios::beg);

	Bytes.resize(Len);
	In.read((char*)Bytes.data(), Len);
	In.close();
	
	return Bytes;
}
