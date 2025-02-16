#include "LoggerService.h"

using namespace Logging;

std::string LoggerService::GetTimeAsString(int Format)
{

	char buffer[80];

	struct tm timeinfo;
	time_t now = time(0);
	localtime_s(&timeinfo, &now);

	if(Format == 1)
		strftime(buffer, sizeof(buffer), "%d-%m-%Y %H:%M:%S", &timeinfo);
	else if(Format == 2)
		strftime(buffer, sizeof(buffer), "%d-%m-%Y", &timeinfo);

	std::string str(buffer);

	return buffer;

}

void LoggerService::WriteToLog(std::string Message, std::string LogType, const char* caller)
{

	Logging::LogMutex.lock();

	Logging::LogFile << "[" << LogType << "]";
	Logging::LogFile << "[" << LoggerService::GetTimeAsString(1) << "]";
	
	if (caller != "")
		Logging::LogFile << "[" << caller << "]";

	Logging::LogFile << " " << Message << std::endl;

	Logging::LogMutex.unlock();

}

void LoggerService::StartLoggerService()
{

	char* appdata = nullptr;
	size_t sz = 0;

	_dupenv_s(&appdata, &sz, "APPDATA");

	std::string str(appdata);

	std::string filepath = "\\BOTWM\\LatestLog.txt";

	std::ifstream file(appdata + filepath);

	if (file.good())
	{
		std::string LogsPath = "\\BOTWM\\Logs";

		std::ifstream LogsFolder(appdata + LogsPath);

		if (!LogsFolder.good())
		{
			_mkdir((appdata + LogsPath).c_str());
		}

		int counter = 0;

		do 
		{

			std::string fileDate;

			std::getline(file, fileDate);

			file.close();

			file.open(appdata + filepath);

			std::ifstream Logfile(appdata + LogsPath + "\\" + fileDate + "_" + std::to_string(counter) + ".txt");

			if (!Logfile.good())
			{

				std::ofstream OldLog((appdata + LogsPath + "\\" + fileDate + "_" + std::to_string(counter) + ".txt").c_str());

				std::string newLine;

				while (std::getline(file, newLine))
				{
					OldLog << newLine << std::endl;
				}

				OldLog.close();
				file.close();

				remove((appdata + filepath).c_str());

				break;

			}

			counter++;

		} while (true);

	}

	file.close();

	LogFile.open((appdata + filepath).c_str(), std::ios_base::app);
	LogFile << LoggerService::GetTimeAsString(2) << std::endl;
	LogFile << "Logging level: ";

	switch (LEVEL)
	{
	case 0:
		LogFile << "Information" << std::endl;
		break;
	case 1:
		LogFile << "Warning" << std::endl;
		break;
	case 2:
		LogFile << "Debug" << std::endl;
		break;
	}

}

void LoggerService::LogWarning(std::string Message, const char* caller)
{
	if (LEVEL < 1)
		return;

	LoggerService::WriteToLog(Message, "WRN", caller);
}

void LoggerService::LogDebug(std::string Message, const char* caller)
{
	if (LEVEL < 2)
		return;
	LoggerService::WriteToLog(Message, "DBG", caller);
}

void LoggerService::LogInformation(std::string Message, const char* caller)
{
	LoggerService::WriteToLog(Message, "INF", caller);
}

void LoggerService::LogCritical(std::string Message, const char* caller)
{
	LoggerService::WriteToLog(Message, "CRT", caller);
}

void LoggerService::LogError(std::string Message, const char* caller)
{
	LoggerService::WriteToLog(Message, "ERR", caller);
}

void LoggerService::StartTimer(std::string timerName)
{
	if (TimerName != "")
		LogWarning("Overwriting timer: \"" + TimerName + "\". Timer results may be wrong.");

	Timer = GetTickCount();
	TimerName = timerName;
}

void LoggerService::FinishTimer()
{
	if (TimerName != "")
		LogDebug("Timer \"" + TimerName + "\" finished at: " + std::to_string(float(GetTickCount() - Timer)) + " ms.");
	else
		LogWarning("Tried to terminate a timer but no timer was set.");

	TimerName = "";
}