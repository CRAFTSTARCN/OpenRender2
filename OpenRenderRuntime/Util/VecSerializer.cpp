#include "VecSerializer.h"

#include "Logger.h"

float VecSerializer::AnaComponent(Json Param, const std::string& Component, bool Log, const std::string& LogInfo)
{
	
	Json Comp = Param[Component];
	
	if(Comp.is_number())
	{
		return (float)Comp.number_value();
	}

	if(Log)
	{
		LOG_WARN_FUNCTION("{0}: Component {1} missing, using 0.0 as default", LogInfo.c_str(), Component.c_str());
	}
	
	return 0.0f;
	
}

glm::vec3 VecSerializer::SerializeVec3(Json VecJson, bool Log, const std::string& LogInfo)
{
	glm::vec3 Res;
	Res.x = AnaComponent(VecJson,"X", Log, LogInfo);
	Res.y = AnaComponent(VecJson,"Y", Log, LogInfo);
	Res.z = AnaComponent(VecJson,"Z", Log, LogInfo);

	return Res;
}

glm::vec4 VecSerializer::SerializeColor(Json VecJson, bool Log, const std::string& LogInfo)
{
	glm::vec4 Res {};
	Res.r = AnaComponent(VecJson, "R", Log, LogInfo);
	Res.g = AnaComponent(VecJson, "G", Log, LogInfo);
	Res.b = AnaComponent(VecJson, "B", Log, LogInfo);
	Res.a = AnaComponent(VecJson, "A", Log, LogInfo);

	return Res;
}

