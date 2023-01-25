#include "Semaphore.h"

Semaphore::Semaphore(int InCount) : Count(InCount) 
{
}

Semaphore::~Semaphore()
{
}

void Semaphore::Signal()
{
    std::unique_lock<std::mutex> Lock(Mutex);
    Count++;
    if(Count == 0)
    {
        Cv.notify_one();
    }
}

void Semaphore::Wait()
{
    std::unique_lock<std::mutex> Lock(Mutex);
    Count--;
    if(Count < 0)
    {
        Cv.wait(Lock);
    }
}