#ifndef LOGGER_HPP
#define LOGGER_HPP

#include <string>
#include <iostream>
#include <iomanip>

enum class LogLevel {
	trace,
	debug,
	info,
	warn,
	error,
	off
};

constexpr LogLevel GlobalLogLevel = LogLevel::trace;

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
				case LogLevel::trace: levelStr = "\033[1;35mTRACE\033[0m"; break;
				case LogLevel::debug: levelStr = "\033[1;34mDEBUG\033[0m"; break;
				case LogLevel::info:  levelStr = "\033[1;37mINFO\033[0m"; break;
				case LogLevel::warn:  levelStr = "\033[1;33mWARN\033[0m"; break;
				case LogLevel::error: levelStr = "\033[1;31mERROR\033[0m"; break;
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