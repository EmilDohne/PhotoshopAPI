#pragma once

#include "../Macros.h"

#include <vector>
#include <chrono>
#include <memory>
#include <iostream>

#include <cstdarg>
#include <stdexcept>


#define PSAPI_LOG(task, format, ...)						NAMESPACE_PSAPI::Logger::getInstance().log(NAMESPACE_PSAPI::Enum::Severity::Info, task, format, __VA_ARGS__);
#define PSAPI_LOG_ERROR(task, format, ...)					NAMESPACE_PSAPI::Logger::getInstance().log(NAMESPACE_PSAPI::Enum::Severity::Error, task, format, __VA_ARGS__);
#define PSAPI_LOG_WARNING(task, format, ...)				NAMESPACE_PSAPI::Logger::getInstance().log(NAMESPACE_PSAPI::Enum::Severity::Warning, task, format, __VA_ARGS__);
#define PSAPI_LOG_DEBUG(task, format, ...)					NAMESPACE_PSAPI::Logger::getInstance().log(NAMESPACE_PSAPI::Enum::Severity::Debug, task, format, __VA_ARGS__);

#define PSAPI_SET_SEVERITY_INFO								NAMESPACE_PSAPI::Logger::getInstance().setSeverity(NAMESPACE_PSAPI::Enum::Severity::Info);
#define PSAPI_SET_SEVERITY_WARNING							NAMESPACE_PSAPI::Logger::getInstance().setSeverity(NAMESPACE_PSAPI::Enum::Severity::Warning);
#define PSAPI_SET_SEVERITY_ERROR							NAMESPACE_PSAPI::Logger::getInstance().setSeverity(NAMESPACE_PSAPI::Enum::Severity::Error);
#define PSAPI_SET_SEVERITY_DEBUG							NAMESPACE_PSAPI::Logger::getInstance().setSeverity(NAMESPACE_PSAPI::Enum::Severity::Debug);
#define PSAPI_SET_SEVERITY_PROFILE							NAMESPACE_PSAPI::Logger::getInstance().setSeverity(NAMESPACE_PSAPI::Enum::Severity::Profile);


PSAPI_NAMESPACE_BEGIN


namespace
{
    std::string leftAlignString(const std::string str, int totalLength)
    {
        int spacesToFill = totalLength - static_cast<int>(str.length());
        if (spacesToFill <= 0)
        {
            return str;
        };
        std::string leftStr = str + std::string(spacesToFill, ' ');

        return leftStr;
    }
}


namespace Enum
{
    enum class Severity
    {
        Info = 0,
        Warning = 1,
        Error = 2,
        Debug = -1,
        Profile = -2
    };
}


class Logger {

public:
    // Function to get the logger instance
    inline static Logger& getInstance() {
        static Logger instance; // This will be created only once
        return instance;
    }

    // Log a message with the given severity, task, and format
    inline void log(Enum::Severity severity, const char* task, const char* format, ...) {

        // parse the format string to a char buffer
        char buffer[1024];
        {
            va_list args;
            va_start(args, format);
            int result = vsnprintf(buffer, sizeof(buffer), format, args);
            va_end(args);
        }
        auto time = this->getTime();
        auto logMessage = this->createMessage(time, task, buffer, severity);

        {
            std::lock_guard<std::mutex> lock(m_mutex);
            if (severity >= m_Severity)
            {
                if (severity == Enum::Severity::Error)
                {
                    std::cout << logMessage << std::endl;
                    throw std::runtime_error(logMessage);
                }
                std::cout << logMessage << std::endl;
            }
        }
    }


    inline void setSeverity(Enum::Severity severity)
    {
        this->m_Severity = severity;
    }

private:
    Enum::Severity m_Severity = Enum::Severity::Debug;
    std::mutex m_mutex; // Mutex for thread safety

    Logger() {
    }

    ~Logger() {
    }


    // Prevent copy and assignment
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;

    inline const std::string getTime()
    {
        auto const time = std::chrono::current_zone()
            ->to_local(std::chrono::system_clock::now());
        return std::format("{:%Y-%m-%d %X}", time);
    }


    inline const std::string createMessage(std::string time, std::string task, const std::string& message, Enum::Severity severity)
    {
        time = leftAlignString(time, 22);

        task = "[" + task + "]";
        task = leftAlignString(task, 15);

        std::string joinedMessage = time + task + message;

        return joinedMessage;
    }

};


PSAPI_NAMESPACE_END

