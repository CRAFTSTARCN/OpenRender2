#include "OpenRenderRuntime/Util/FileUtil.h"

#include "fstream"

std::string FileUtil::LoadFile2Str(const std::string& Path)
{
	std::ifstream File;
	File.open(Path, std::ios::in);
	std::string Str;

	if(!File.is_open())
	{
		return Str;
	}

	File.seekg(0, std::ios::end);
	std::streamsize Len = File.tellg();
	File.seekg(0, std::ios::beg);

	char* Buffer = new char[Len + 1]{0};
	File.read(Buffer, Len);
	File.close();
	Str.assign(Buffer);
	delete[] Buffer;
	
	return  Str;
}

std::vector<std::byte> FileUtil::LoadFile2Byte(const std::string& Path)
{
	std::ifstream In;
	std::vector<std::byte> Bytes;
	In.open(Path, std::ios::in|std::ios::binary);

	if(!In.is_open())
	{
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
