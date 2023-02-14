#pragma once
#include <cstdint>
#include <mutex>

class Barrier
{
	uint32_t TotalThread;
	volatile uint32_t Count;
	volatile uint32_t Reach;

	std::mutex Mute;
	std::condition_variable CV;
	
public:

	Barrier(uint32_t Total);

	void Wait();
};
