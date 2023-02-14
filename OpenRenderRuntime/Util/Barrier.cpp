#include "Barrier.h"

#include "Logger.h"
Barrier::Barrier(uint32_t Total) : TotalThread(Total), Count(0), Reach(0)
{
}

void Barrier::Wait()
{
	std::unique_lock<std::mutex> ScopeLock(Mute);
	if(Count == 0)
	{
		Reach = 0;
	}
	++Count;
	if(Count == TotalThread)
	{
		Reach = 1;
		Count = 0;
		CV.notify_all();
	}
	else
	{
		CV.wait(ScopeLock);
	}
}
