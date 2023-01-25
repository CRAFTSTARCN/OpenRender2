#pragma once
#include <condition_variable>

class Semaphore
{

    std::condition_variable Cv;
    std::mutex Mutex;
    int Count;
    
public:

    Semaphore(int InCount = 0);
    ~Semaphore();

    Semaphore(const Semaphore&) = delete;
    Semaphore(Semaphore&&) = delete;
    Semaphore& operator=(const Semaphore&) = delete;
    Semaphore& operator=(Semaphore&&) = delete;

    void Signal();
    
    void Wait();
    
    
};
