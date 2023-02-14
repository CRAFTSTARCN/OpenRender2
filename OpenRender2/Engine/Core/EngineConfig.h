#pragma once
#include "OpenRenderRuntime/Modules/RenderConfig/RenderCofig.h"

class EngineConfig
{

	EngineConfig();
	
public:

	std::string ContentFolder; //Root working dir
	std::string ProjectFilePath;
	
	EngineConfig(const EngineConfig&) = delete;
	EngineConfig(EngineConfig&&) = delete;

	EngineConfig& operator=(const EngineConfig&) = delete;
	EngineConfig& operator=(EngineConfig&&) = delete;

	RenderConfig& GetRenderConfig();

	void ReadConfig(const std::string& Path);
	
	static EngineConfig& Get();
	
};
