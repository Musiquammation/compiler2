#pragma once

#include <cstdarg>
#include <string>

enum class LoggerLevel {
	DEBUG,
	INFO,
	WARN,
	ERROR
};

class Logger {
public:
	Logger(const std::string& key, LoggerLevel level);

	void debug(const char* format, ...);
	void info(const char* format, ...);
	void warn(const char* format, ...);
	void error(const char* format, ...);

private:
	std::string key;
	LoggerLevel level;

	// Log a printf-style formatted message.
	void log(
		LoggerLevel messageLevel,
		const char* levelName,
		const char* format,
		va_list args
	);
};

// Get a logger using the level configured for its key.
Logger getLogger(const std::string& key);