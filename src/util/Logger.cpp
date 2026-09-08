#include "Logger.hpp"
#include "DEFAULT_LOGGER_LEVELS.hpp"

#include <cstdio>
#include <fstream>
#include <iostream>
#include <vector>

#define LOGS_PATH "dist/logs.log"


namespace {
	struct TabulatorState {
		std::size_t ownerIndex;
		int tabs;
	};

	thread_local std::vector<TabulatorState> tabulatorStack;
}

static std::string getTabulation() {
	if (tabulatorStack.empty())
		return "";

	return std::string(tabulatorStack.back().tabs, '\t');
}


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


void Logger::debug(std::string data) {
	log(LoggerLevel::DEBUG, "DEBUG", data);
}

void Logger::info(std::string data) {
	log(LoggerLevel::INFO, "INFO", data);
}

void Logger::warn(std::string data) {
	log(LoggerLevel::WARN, "WARN", data);
}

void Logger::error(std::string data) {
	log(LoggerLevel::ERROR, "ERROR", data);
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
		getTabulation() +
		"[" + std::string(levelName) + "] " +
		key + ": " + buffer.data();

	// Write to the console.
	std::cout << message << std::endl;

	// Append to the log file.
	std::ofstream file(LOGS_PATH, std::ios::app);

	if (file.is_open())
		file << message << std::endl;
}


void Logger::log(
	LoggerLevel messageLevel,
	const char* levelName,
	const std::string& data
) {
	// Ignore messages below the configured log level.
	if (static_cast<int>(messageLevel) < static_cast<int>(level))
		return;

	const std::string message =
		getTabulation() +
		"[" + std::string(levelName) + "] " +
		key + ": " + data;

	// Write to the console.
	std::cout << message << std::endl;

	// Append to the log file.
	std::ofstream file(LOGS_PATH, std::ios::app);

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




Logger::Tabulator Logger::tabulator() {
	int tabs = tabulatorStack.empty() ? 0 : tabulatorStack.back().tabs;
	return Tabulator(tabs);
}



Logger::Tabulator::Tabulator(int tabs)
	: stackIndex(tabulatorStack.size()),
	  active(true) {

	tabulatorStack.push_back({
		stackIndex,
		tabs
	});
}


Logger::Tabulator::Tabulator(Tabulator&& other) noexcept
	: stackIndex(other.stackIndex),
	  active(other.active) {

	other.active = false;
}


Logger::Tabulator& Logger::Tabulator::operator=(Tabulator&& other) noexcept {
	if (this == &other)
		return *this;

	if (active) {
		// A moved-from object should normally be inactive.
		// Do not silently corrupt the stack.
		if (stackIndex + 1 == tabulatorStack.size())
			tabulatorStack.pop_back();
	}

	stackIndex = other.stackIndex;
	active = other.active;

	other.active = false;

	return *this;
}


Logger::Tabulator::~Tabulator() {
	if (!active)
		return;

	// The tabulator must still be the top of the stack.
	if (stackIndex + 1 != tabulatorStack.size()) {
		// This should only happen if the lifetime order is violated.
		std::terminate();
	}

	tabulatorStack.pop_back();
}


void Logger::Tabulator::add() {
	if (!active)
		throw std::logic_error("Attempted to use an inactive tabulator");

	if (stackIndex + 1 != tabulatorStack.size())
		throw std::logic_error(
			"Attempted to use an inactive tabulator while another tabulator is active"
		);

	++tabulatorStack.back().tabs;
}


void Logger::Tabulator::remove() {
	if (!active)
		throw std::logic_error("Attempted to use an inactive tabulator");

	if (stackIndex + 1 != tabulatorStack.size())
		throw std::logic_error(
			"Attempted to use an inactive tabulator while another tabulator is active"
	);

	if (tabulatorStack.back().tabs > 0)
		--tabulatorStack.back().tabs;
}


void Logger::Tabulator::reset() {
	if (!active)
		throw std::logic_error("Attempted to use an inactive tabulator");

	if (stackIndex + 1 != tabulatorStack.size())
		throw std::logic_error(
			"Attempted to use an inactive tabulator while another tabulator is active"
	);

	tabulatorStack.back().tabs = 0;
}

