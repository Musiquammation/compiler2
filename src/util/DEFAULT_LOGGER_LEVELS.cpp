#include "DEFAULT_LOGGER_LEVELS.hpp"

const std::unordered_map<std::string, LoggerLevel> DEFAULT_LOGGER_LEVELS = {
	{"default", LoggerLevel::INFO},
	{"main", LoggerLevel::DEBUG},
	{"class", LoggerLevel::INFO},
	{"prototype", LoggerLevel::INFO},
	{"variable", LoggerLevel::INFO},
	{"procres", LoggerLevel::INFO},
	{"procedures-variadic", LoggerLevel::INFO},
	{"procedures-friendVariadic", LoggerLevel::INFO},
	{"procedures-metaVariadic", LoggerLevel::INFO},
};

