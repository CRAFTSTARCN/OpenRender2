#pragma once
#include <cstdint>
#include <mutex>

class Barrier
{
	uint32_t TotalThread;
	uint32_t Count;
	uint32_t Reach;

	std::mutex Mute;
	std::condition_variable CV;
	
public:

	Barrier(uint32_t Total);

	void Wait();
};
