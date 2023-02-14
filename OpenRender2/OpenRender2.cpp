#include "OpenRenderRuntime/Util/Logger.h"

#include <filesystem>

#include "Engine/Core/EngineConfig.h"
#include "Engine/Core/OpenRenderEngine.h"
#include "OpenRenderRuntime/Core/Basic/RenderMath.h"
#include "OpenRenderRuntime/Core/Render/Render.h"
#include "OpenRenderRuntime/Modules/AssetSystem/AssetSystem.h"
#include "OpenRenderRuntime/Modules/InputSystem/InputSystem.h"
#include "OpenRenderRuntime/Modules/RenderConfig/RenderCofig.h"
#include "OpenRenderRuntime/Modules/RenderData/RenderableInstanceData.h"
#include "OpenRenderRuntime/Modules/RenderData/SceneSwapData.h"


int main(int argc, char* argv[])
{
    /*
     * Path enviroment setting
     */
    std::filesystem::path ExeDir = std::filesystem::path(argv[0]).parent_path();
    LOG_INFO_FUNCTION("Open Render 2 Running");
    LOG_INFO_FUNCTION("Base dir: {}", ExeDir.generic_string());

    RenderConfig::Get().RootDir = ExeDir.generic_string();

	if(argc > 1)
	{
		EngineConfig::Get().ProjectFilePath = argv[1];
	}
	else
	{
		EngineConfig::Get().ProjectFilePath = (ExeDir / "Project.json").generic_string();
	}
	LOG_INFO_FUNCTION("Project path: {0}", EngineConfig::Get().ProjectFilePath.c_str());


	GEngine = new OpenRenderEngine;
	GEngine->LoadProject(EngineConfig::Get().ProjectFilePath);
	GEngine->PreInit();
	GEngine->Init();
	GEngine->EngineLoop();
	GEngine->Terminate();
    return 0;
}
