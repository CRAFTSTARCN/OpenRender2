#include "OpenRender2/Engine/Core/EngineConfig.h"

EngineConfig::EngineConfig()
{
}

RenderConfig& EngineConfig::GetRenderConfig()
{
	return RenderConfig::Get();
}

void EngineConfig::ReadConfig(const std::string& Path)
{
	//TODO:
}

EngineConfig& EngineConfig::Get()
{
	static EngineConfig Config;
	return Config;
}
