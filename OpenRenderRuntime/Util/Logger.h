#pragma once
#include <string>
#include "spdlog/logger.h"

#define LOG_FUNCTION_PREFIX (std::string("[") + std::string(__FUNCTION__) + std::string("] "))

#define LOG_INFO(...) Logger::Get().Info(__VA_ARGS__)
#define LOG_INFO_FUNCTION(...) Logger::Get().Info(LOG_FUNCTION_PREFIX + __VA_ARGS__)

#define LOG_DEBUG(...) Logger::Get().Debug(__VA_ARGS__)
#define LOG_DEBUG_FUNCTION(...) Logger::Get().Debug(LOG_FUNCTION_PREFIX + __VA_ARGS__)

#define LOG_WARN(...) Logger::Get().Warn(__VA_ARGS__)
#define LOG_WARN_FUNCTION(...) Logger::Get().Warn(LOG_FUNCTION_PREFIX + __VA_ARGS__)

#define LOG_ERROR(...) Logger::Get().Error(__VA_ARGS__)
#define LOG_ERROR_FUNCTION(...) Logger::Get().Error(LOG_FUNCTION_PREFIX + __VA_ARGS__)

#define LOG_FATAL(...) Logger::Get().Fatal(__VA_ARGS__)
#define LOG_FATAL_FUNCTION(...) Logger::Get().Fatal(LOG_FUNCTION_PREFIX + __VA_ARGS__)

class Logger final
{
    Logger();
    std::shared_ptr<spdlog::logger> SpdLoggerPtr;
    
public:

    /*
     * Configures, will expire after logger global object created
     */
    inline static std::string FileName = std::string();
    inline static size_t TaskQueueSize = 4096;
    inline static size_t LogThreadCount = 1;
    inline static bool LogToFile = false;

    
    ~Logger();
    
    Logger(const Logger&) = delete;
    Logger(Logger&&) = delete;
    Logger& operator=(const Logger&) = delete;
    Logger& operator=(Logger&&) = delete;

    template <typename... VARGT>
    void Info(VARGT&& ...Args)
    {
        SpdLoggerPtr->info(std::forward<VARGT>(Args)...);
    }

    template <typename... VARGT>
    void Debug(VARGT&& ...Args)
    {
        SpdLoggerPtr->debug(std::forward<VARGT>(Args)...);
    }

    template <typename... VARGT>
    void Warn(VARGT&& ...Args)
    {
        SpdLoggerPtr->warn(std::forward<VARGT>(Args)...);
    }

    template <typename... VARGT>
    void Error(VARGT&& ...Args)
    {
        SpdLoggerPtr->error(std::forward<VARGT>(Args)...);
    }

    template <typename... VARGT>
    void Fatal(VARGT&& ...Args)
    {
        SpdLoggerPtr->critical(std::forward<VARGT>(Args)...);
        SpdLoggerPtr->flush();
        assert(false);
    }

    static Logger& Get();
};
