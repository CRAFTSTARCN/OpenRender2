#pragma once
#include <string>
#include <unordered_map>

struct AssetSystemConfig
{
	std::string BasePath;
	std::string ShaderPlatform;

	std::unordered_map<std::string, std::string> CustomConfig;
};
