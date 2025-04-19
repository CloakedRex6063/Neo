#pragma once

#include "Tools/Tools.hpp"
#include "spdlog/spdlog.h"


namespace Neo
{
/// <summary>
/// Logger class
/// </summary>
class Log
{
public:
    /// <summary>
    /// Initialize the logger, setting the pattern and the level of logging.
    /// </summary>
    static void Init();

    /// <summary>
    /// Info level logging.
    /// </summary>
    /// <param name="fmt">Format string, using Python-like format string syntax.</param>
    /// <param name="args"></param>
    ///	<param name="...args">List of positional arguments.</param>
    template <typename... Args>
    static constexpr void Info(const fmt::format_string<Args...>& fmt, Args&&... args);

    /// <summary>
    /// Warning level logging. Engine can still run as intended.
    /// </summary>
    /// <param name="fmt">Format string, using Python-like format string syntax.</param>
    /// <param name="args"></param>
    ///	<param name="...args">List of positional arguments.</param>
    template <typename... Args>
    static constexpr void Warn(const fmt::format_string<Args...>& fmt, Args&&... args);

    /// <summary>
    /// Error level logging. Engine may still be able to run, but not as intended.
    /// </summary>
    /// <param name="fmt">Format string, using Python-like format string syntax.</param>
    /// <param name="args"></param>
    /// <param name="...args">List of positional arguments.</param>
    template <typename... Args>
    static constexpr void Error(const fmt::format_string<Args...>& fmt, Args&&... args);

    /// <summary>
    /// Critical level logging. Engine cannot run.
    /// </summary>
    /// <param name="fmt">Format string, using Python-like format string syntax.</param>
    /// <param name="args"></param>
    ///	<param name="...args">List of positional arguments.</param>
    template <typename... Args>
    static constexpr void Critical(const fmt::format_string<Args...>& fmt, Args&&... args);
};

template <typename... Args>
constexpr void Log::Info(const fmt::format_string<Args...>& fmt, Args&&... args)
{
    spdlog::info(fmt, std::forward<Args>(args)...);
}

template <typename... Args>
constexpr void Log::Warn(const fmt::format_string<Args...>& fmt, Args&&... args)
{
    spdlog::warn(fmt, std::forward<Args>(args)...);
}

template <typename... Args>
constexpr void Log::Error(const fmt::format_string<Args...>& fmt, Args&&... args)
{
    spdlog::error(fmt, std::forward<Args>(args)...);
}

template <typename... Args>
constexpr void Log::Critical(const fmt::format_string<Args...>& fmt, Args&&... args)
{
    spdlog::critical(fmt, std::forward<Args>(args)...);
    const auto formattedString = FormatString(fmt, std::forward<Args>(args)...);
    ThrowError(formattedString);
}

}  // namespace bee