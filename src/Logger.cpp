#include "Logger.hpp"
#include "DEFAULT_LOGGER_LEVELS.hpp"

#include <cstdio>
#include <fstream>
#include <iostream>
#include <vector>

Logger::Logger(const std::string& key, LoggerLevel level)
	: key(key), level(level) {}

void Logger::debug(const char* format, ...) {
	va_list args;
	va_start(args, format);

	log(LoggerLevel::DEBUG, "DEBUG", format, args);

	va_end(args);
}

void Logger::info(const char* format, ...) {
	va_list args;
	va_start(args, format);

	log(LoggerLevel::INFO, "INFO", format, args);

	va_end(args);
}

void Logger::warn(const char* format, ...) {
	va_list args;
	va_start(args, format);

	log(LoggerLevel::WARN, "WARN", format, args);

	va_end(args);
}

void Logger::error(const char* format, ...) {
	va_list args;
	va_start(args, format);

	log(LoggerLevel::ERROR, "ERROR", format, args);

	va_end(args);
}

void Logger::log(
	LoggerLevel messageLevel,
	const char* levelName,
	const char* format,
	va_list args
) {
	// Ignore messages below the configured log level.
	if (static_cast<int>(messageLevel) < static_cast<int>(level))
		return;

	// Make a copy because va_list can only safely be consumed once.
	va_list argsCopy;
	va_copy(argsCopy, args);

	int size = std::vsnprintf(nullptr, 0, format, argsCopy);
	va_end(argsCopy);

	if (size < 0)
		return;

	std::vector<char> buffer(size + 1);

	std::vsnprintf(buffer.data(), buffer.size(), format, args);

	const std::string message =
		"[" + std::string(levelName) + "] " + key + ": " + buffer.data();

	// Write to the console.
	std::cout << message << std::endl;

	// Append to the log file.
	std::ofstream file("dist/logs.log", std::ios::app);

	if (file.is_open())
		file << message << std::endl;
}

Logger getLogger(const std::string& key) {
	auto it = DEFAULT_LOGGER_LEVELS.find(key);

	if (it != DEFAULT_LOGGER_LEVELS.end())
		return Logger(key, it->second);

	// Use the default level for unknown keys.
	auto defaultIt = DEFAULT_LOGGER_LEVELS.find("default");

	if (defaultIt != DEFAULT_LOGGER_LEVELS.end())
		return Logger(key, defaultIt->second);

	return Logger(key, LoggerLevel::INFO);
}
