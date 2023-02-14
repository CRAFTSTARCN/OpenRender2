#include "OpenRender2/Engine/Core/OpenRenderEngine.h"

#include <filesystem>

#include "EngineConfig.h"
#include "OpenRenderRuntime/Modules/RenderData/SceneSwapData.h"
#include "OpenRenderRuntime/Util/FileUtil.h"

OpenRenderEngine* GEngine = nullptr;

OpenRenderEngine::OpenRenderEngine()
{
	RenderSystem = new Render;
	SwapDataCenter = new RenderSwapDataCenter;
	Input = new InputSystem;
	Client = new ViewportClient;
	AssetSys = new AssetSystem;
}

OpenRenderEngine::~OpenRenderEngine()
{
	delete RenderSystem;
	delete SwapDataCenter;
	delete Input;
	delete Client;
	delete AssetSys;
}

InputSystem* OpenRenderEngine::GetInputSystem()
{
	return Input;
}

RenderSwapDataCenter* OpenRenderEngine::GetSwapDataCenter()
{
	return SwapDataCenter;
}

LLevel* OpenRenderEngine::GetCurrentLevel()
{
	return CurrentLevel;
}

AssetSystem* OpenRenderEngine::GetAssetSystem()
{
	return AssetSys;
}

ViewportClient* OpenRenderEngine::GetClient()
{
	return Client;
}

void OpenRenderEngine::LoadProject(const std::string& ProjectFullPath)
{
	std::string ProjFile = FileUtil::LoadFile2Str(ProjectFullPath);
	if(ProjFile.empty())
	{
		LOG_ERROR_FUNCTION("Fail to open project file at {0}", ProjectFullPath.c_str());
		assert(false);
	}

	std::string Err;
	Json ProjJson = Json::parse(ProjFile, Err);
	if(ProjJson.is_null())
	{
		LOG_ERROR_FUNCTION("Fail to parse project json {0}, err: {1}", ProjectFullPath.c_str(), Err.c_str());
		assert(false);
	}

	std::filesystem::path ProjectFileDir = std::filesystem::path(ProjectFullPath).parent_path();

	if(ProjectJson["ContentFolder"].is_string())
	{
		EngineConfig::Get().ContentFolder = (ProjectFileDir / ProjectJson["ContentFolder"].string_value()).generic_string();
	}
	else
	{
		EngineConfig::Get().ContentFolder = (ProjectFileDir / "Content").generic_string();
	}
	
	ProjectJson = ProjJson;
}

void OpenRenderEngine::OpenLevel(const std::string& LevelPath)
{
	size_t AssetId = AssetSys->GetOrImport(LevelPath);

	if(AssetId == AssetRegistry::BAD_GASSET_ID)
	{
		LOG_ERROR_FUNCTION("Fail to read level asset {0}", LevelPath.c_str());
		return;
	}

	AssetObject* RowAsset = AssetSys->GetAssetById(AssetId);
	LevelAsset* Asset = dynamic_cast<LevelAsset*>(RowAsset);
	if(!Asset)
	{
		LOG_ERROR_FUNCTION("Invalid asset type, require level got {0}, path {1}", RowAsset->GetAssetTypeName().c_str(), LevelPath.c_str());
	}

	if (CurrentLevel)
	{
		CurrentLevel->Unload();
		delete CurrentLevel;
	}

	CurrentLevel = new LLevel;
	CurrentLevel->SerializeFromAsset(Asset);
}

void OpenRenderEngine::PreInit()
{
	RenderSystem->PreInit();
}

void OpenRenderEngine::Init()
{
	RenderThread.Run(&Render::Init, RenderSystem, SwapDataCenter);

	Client->Init(&RenderSystem->GetWindow());
	
	AssetSys->Init({
		EngineConfig::Get().ContentFolder,
		RenderSystem->GetRenderRHIShaderPlatform(),
		SwapDataCenter,
		false});

	Input->Init(&RenderSystem->GetWindow());

	const std::string& BRDFLUTPath = ProjectJson["BRDFLUT"].string_value();
	if(BRDFLUTPath.empty())
	{
		LOG_ERROR_FUNCTION("No BRDF LUT path in project, fatal");
		assert(false);
	}

	size_t BRDFLUTAssetId = AssetSys->Import(BRDFLUTPath);
	assert(BRDFLUTAssetId != AssetRegistry::BAD_GASSET_ID);
	assert(dynamic_cast<TextureAsset*>(AssetSys->GetAssetById(BRDFLUTAssetId)));

	const std::string& DefaultLevelPath = ProjectJson["DefaultLevel"].string_value();
	if(DefaultLevelPath.empty())
	{
		LOG_ERROR_FUNCTION("No default level path in project, fatal");
		assert(false);
	}

	OpenLevel(DefaultLevelPath);
	assert(CurrentLevel);

	RenderThread.WaitForPushable();
	SwapDataCenter->EndLoopSwap();

	RenderThread.Run(&Render::InitialLoading, RenderSystem);

	RenderThread.WaitForPushable();
	SwapDataCenter->EndLoopSwap();
	EnvironmentTextureData *BRDFLUTSetter = new EnvironmentTextureData;
	BRDFLUTSetter->BRDFLUTId = dynamic_cast<TextureAsset*>(AssetSys->GetAssetById(BRDFLUTAssetId))->GetTexId();
	SwapDataCenter->GetLogicSide().push_back(BRDFLUTSetter);
}

void OpenRenderEngine::EngineLoop()
{
	
	CurrentLevel->Begin();
	SwapDataCenter->EndLoopSwap();
	
	auto RenderThreadFunc = [this]()
	{
		while(!RenderSystem->GetWindow().WindowShouldClose())
		{
			this->LogicRenderBarrier.Wait();
			RenderSystem->Tick(this->Timer.GetDeltaTime());
			this->LogicRenderBarrier.Wait();
		}
	};

	RenderThread.Run(RenderThreadFunc);
	while(!Client->ClientShutdown())
	{
		Timer.CalculateTime();
		
		LogicRenderBarrier.Wait();
		Input->Tick(Timer.GetDeltaTime());
		CurrentLevel->Tick(Timer.GetDeltaTime());
		LogicRenderBarrier.Wait();

		SwapDataCenter->EndLoopSwap();
	}
	RenderThread.WaitForPushable();
	
}

void OpenRenderEngine::Terminate()
{
	RenderThread.Run(&Render::Terminate, RenderSystem);
	RenderThread.WaitForPushable();
	AssetSys->Terminate();
}
