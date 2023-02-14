#pragma once
#include <string>
#include <vector>

class FileUtil
{
	
public:
	static std::string LoadFile2Str(const std::string& Path);

	static std::vector<std::byte> LoadFile2Byte(const std::string& Path);
};
