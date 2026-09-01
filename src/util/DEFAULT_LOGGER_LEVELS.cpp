#include "DEFAULT_LOGGER_LEVELS.hpp"

const std::unordered_map<std::string, LoggerLevel> DEFAULT_LOGGER_LEVELS = {
	{"default", LoggerLevel::INFO},
	{"main", LoggerLevel::DEBUG},
	{"class", LoggerLevel::DEBUG},
};

