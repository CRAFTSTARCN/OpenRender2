#pragma once

#include <functional>
#include <future>
#include <thread>

#include "OpenRenderRuntime/Util/Semaphore.h"

class MultiTimeRunable
{

    bool Running;
    Semaphore Fetch;
    Semaphore Push;
    std::shared_ptr<std::function<void()>> FuncPtr;    
    std::thread RawThread;

    void TemplateFunction()
    {
        while(Running)
        {
            Fetch.Wait();
            if(FuncPtr)
            {
                (*FuncPtr)();
                FuncPtr.reset();
                Push.Signal();
            }
        }
    }
    
public:

    MultiTimeRunable() : Running(true), Fetch(0), Push(1), FuncPtr(nullptr), RawThread([this](){ this->TemplateFunction(); })
    {
        
    }

    ~MultiTimeRunable()
    {
        Push.Wait();
        Running = false;
        Fetch.Signal();
        RawThread.join();
    }

    MultiTimeRunable(const MultiTimeRunable&&) = delete;
    MultiTimeRunable(MultiTimeRunable&&) = delete;

    MultiTimeRunable& operator=(const MultiTimeRunable&) = delete;
    MultiTimeRunable& operator=(MultiTimeRunable&&) = delete;

    template<typename FT, typename... ArgT>
    auto Run(FT&& Func, ArgT ...Args)
    {
        Push.Wait();
        auto Ptr = std::make_shared<std::packaged_task<std::invoke_result_t<FT, ArgT...>()>>(std::bind(std::forward<FT>(Func), std::forward<ArgT>(Args)...));
        FuncPtr = std::make_shared<std::function<void()>>([Ptr]()
        {
            (*Ptr)();
        });
        Fetch.Signal();
        
        return Ptr->get_future();
    }
    
    void WaitForPushable()
    {
        Push.Wait();
        Push.Signal();
    }
};
