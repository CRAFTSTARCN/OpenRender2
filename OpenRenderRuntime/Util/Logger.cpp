#include "Logger.h"
#include "spdlog/sinks/basic_file_sink.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/spdlog.h"
#include "spdlog/async.h"

Logger::Logger()
{
    spdlog::init_thread_pool(TaskQueueSize, LogThreadCount);

    if(LogToFile)
    {
        auto SinkFile = std::make_shared<spdlog::sinks::basic_file_sink_mt>(FileName);
        SinkFile->set_level(spdlog::level::trace);
        SinkFile->set_pattern("[%^%l%$] %v");

        spdlog::sinks_init_list InitList = {SinkFile};
        SpdLoggerPtr = std::make_shared<spdlog::async_logger>(
            "OpenRenderLogger",
            InitList.begin(),
            InitList.end(),
            spdlog::thread_pool(),
            spdlog::async_overflow_policy::block);
    }
    else
    {
        auto SinkConsole = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
        SinkConsole->set_level(spdlog::level::trace);
        SinkConsole->set_pattern("[%^%l%$] %v");

        spdlog::sinks_init_list InitList = {SinkConsole};
        SpdLoggerPtr = std::make_shared<spdlog::async_logger>(
            "OpenRenderLogger",
            InitList.begin(),
            InitList.end(),
            spdlog::thread_pool(),
            spdlog::async_overflow_policy::block);
    }

    assert(SpdLoggerPtr);
    SpdLoggerPtr->set_level(spdlog::level::trace);
    spdlog::register_logger(SpdLoggerPtr);
}

Logger::~Logger()
{
    SpdLoggerPtr->flush();
    spdlog::drop_all();
}

Logger& Logger::Get()
{
    static Logger GlobalLogger;
    return GlobalLogger;
}
