/// @file Log.cppm
/// @brief Logging utilities: core logger, client logger, and fatal error handler.
/// @ingroup Core

module;

#include <spdlog/cfg/env.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

export module Nodens.Log;

import std;

namespace Nodens::LogDefaults
{
#if defined(ND_DEBUG)
inline constexpr spdlog::level::level_enum Level = spdlog::level::trace;
#else
inline constexpr spdlog::level::level_enum Level = spdlog::level::info;
#endif
} // namespace Nodens::LogDefaults

export namespace Nodens
{
/// @brief Retrieves the framework's core logger instance.
/// @details This logger is used for internal framework diagnostics and logs under the name
/// "NODENS". It utilizes a thread-safe Meyers' singleton to ensure the logger is only instantiated
/// once on its first invocation.
/// @return A reference to the core spdlog::logger instance.
inline spdlog::logger& CoreLogger()
{
    static std::shared_ptr<spdlog::logger> coreLogger = []()
    {
        spdlog::set_pattern("%^[%T] %n: %v%$");

        std::shared_ptr<spdlog::logger> logger = spdlog::get("NODENS");
        if (!logger)
        {
            logger = spdlog::stdout_color_mt("NODENS");
            logger->set_level(LogDefaults::Level);
        }

        return logger;
    }();

    return *coreLogger;
}

/// @brief Retrieves the client application's logger instance.
/// @details This logger is used for user application logs and logs under the name "APP".
/// It utilizes a thread-safe Meyers' singleton to ensure the logger is only instantiated once
/// on its first invocation.
/// @return A reference to the client spdlog::logger instance.
inline spdlog::logger& ClientLogger()
{
    static std::shared_ptr<spdlog::logger> clientLogger = []()
    {
        spdlog::set_pattern("%^[%T] %n: %v%$");

        std::shared_ptr<spdlog::logger> logger = spdlog::get("APP");
        if (!logger)
        {
            logger = spdlog::stdout_color_mt("APP");
            logger->set_level(LogDefaults::Level);
        }

        return logger;
    }();

    return *clientLogger;
}

/// @brief Explicitly initializes the core and client loggers at startup.
/// @details While loggers are lazily initialized upon their first use, calling this function
/// during application startup forces their instantiation and configuration. This ensures that
/// logging sinks, format patterns, and levels are established on the main thread prior to spawning
/// other threads, avoiding initialization overhead and first-log latency during runtime.
inline void InitializeLoggers()
{
    (void)CoreLogger();
    (void)ClientLogger();
    if (std::getenv("SPDLOG_LEVEL"))
        spdlog::cfg::load_env_levels();
}

/// @brief Logs a critical error message to the core logger and terminates the application.
/// @param message The fatal error message to log.
[[noreturn]] inline void FatalCore(std::string_view message)
{
    CoreLogger().error("{}", message);
    std::terminate();
}
} // namespace Nodens
