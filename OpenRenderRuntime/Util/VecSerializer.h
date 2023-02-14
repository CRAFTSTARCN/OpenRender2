#pragma once

#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <json11.hpp>
#include <glm/glm.hpp>

using json11::Json;

class VecSerializer
{


	static float AnaComponent(Json Param, const std::string& Component, bool Log, const std::string& LogInfo);
	
	
public:

	static glm::vec3 SerializeVec3(Json VecJson, bool Log = false, const std::string& LogInfo = "");

	static glm::vec4 SerializeColor(Json VecJson, bool Log = false, const std::string& LogInfo = "");
	
};
