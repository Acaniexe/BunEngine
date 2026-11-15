#pragma once
#include "Core.h"
#include <fstream>

namespace Bun
{
    enum class LogLevel
    {
        Info,
        Warn,
        Error,
        Fatal
    };

    class Logger
    {
    public:
        static Logger& Get();
        void Log(LogLevel level, const std::string& message);
        void Info(const std::string& message);
        void Warn(const std::string& message);
        void Error(const std::string& message);
        void Fatal(const std::string& message);
        void EnableFileLogging(const std::string& filePath);

    private:
        Logger() = default;
        BUN_NON_COPYABLE_MOVEABLE(Logger)

        std::string _logFilePath;
        std::ofstream _logFile;
        bool _fileLoggingEnabled = false;
    };
}

#define BUN_INFO(...)  ::Bun::Logger::Get().Info (__VA_ARGS__)
#define BUN_WARN(...)  ::Bun::Logger::Get().Warn (__VA_ARGS__)
#define BUN_ERROR(...) ::Bun::Logger::Get().Error(__VA_ARGS__)
#define BUN_FATAL(...) ::Bun::Logger::Get().Fatal(__VA_ARGS__)