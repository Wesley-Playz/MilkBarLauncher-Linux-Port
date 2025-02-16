#pragma once

#include <fstream>
#include <iomanip>
#include <ctime>
#include <string>
#include <iostream>
#include <shared_mutex>
#include <chrono> // For cross-platform timing
#include <filesystem> // For directory creation (C++17)

/*
	LEVEL
	0 = Information
	1 = Warning
	2 = Debug
*/
#define LEVEL 2

namespace Logging
{
    static std::ofstream LogFile;
    static std::chrono::steady_clock::time_point TimerStart; // Cross-platform timer
    static std::string TimerName = "";
    static std::shared_mutex LogMutex;

    class LoggerService // Removed 'static' from here
    {
    private:
        static void WriteToLog(std::string Message, std::string LogType, const char* caller = "");
        static std::string GetTimeAsString(int Format);

    public:
        static void StartLoggerService();
        
        static void LogDebug(std::string Message, const char * caller = "");
        static void LogWarning(std::string Message, const char* caller = "");
        static void LogInformation(std::string Message, const char* caller = "");
        static void LogCritical(std::string Message, const char* caller = "");
        static void LogError(std::string Message, const char* caller = "");

        static void StartTimer(std::string timerName);
        static void FinishTimer();
    };
}