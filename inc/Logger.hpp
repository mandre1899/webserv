#ifndef LOGGER_HPP
#define LOGGER_HPP

#include <string>
#include <iostream>
#include <iomanip>

enum class LogLevel {
	TRACE,
	DEBUG,
	INFO,
	WARN,
	ERROR,
	OFF
};

constexpr LogLevel GlobalLogLevel = LogLevel::TRACE;

class Logger {
private:

public:
	/* Constructors & Destructors */
	Logger() = delete;
	Logger(const Logger& src) = delete;
	Logger(Logger&& src) = delete;
	~Logger() = delete;

	/* Member Functions */
	template<LogLevel level, typename... Args>
	static void log(Args... args)
	{
		if constexpr (level >= GlobalLogLevel)
		{
			const char* levelStr;
			switch (level) {
				case LogLevel::TRACE: levelStr = "\033[1;35mTRACE\033[0m"; break;
				case LogLevel::DEBUG: levelStr = "\033[1;34mDEBUG\033[0m"; break;
				case LogLevel::INFO:  levelStr = "\033[1;37mINFO\033[0m"; break;
				case LogLevel::WARN:  levelStr = "\033[1;33mWARN\033[0m"; break;
				case LogLevel::ERROR: levelStr = "\033[1;31mERROR\033[0m"; break;
				default:              levelStr = "\033[1;33mUNKNOWN\033[0m"; break;
			}
			time_t timestamp = std::time(&timestamp);
			std::cout << std::put_time(std::localtime(&timestamp), "%Y-%m-%d %H:%M:%S") << " [" << levelStr << "] ";
			(std::cout << ... << args) << std::endl;
		}
	}

	/* Basic Operators */
	Logger& operator=(const Logger& src) = delete;
	Logger& operator=(Logger&& src) = delete;

};

#endif // LOGGER_HPP