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
	class Tabulator {
		friend Logger;
		Tabulator(Tabulator&& other) noexcept;
		Tabulator(const Tabulator&) = delete;

		Tabulator& operator=(Tabulator&& other) noexcept;
		Tabulator& operator=(const Tabulator&) = delete;

		
	public:
		void add();
		void remove();
		void reset();
		~Tabulator();

	private:
		friend class Logger;

		explicit Tabulator(int tabs);

		std::size_t stackIndex;
		bool active;
	};

	Logger(const std::string& key, LoggerLevel level);

	void debug(const char* format, ...);
	void info(const char* format, ...);
	void warn(const char* format, ...);
	void error(const char* format, ...);

	void debug(std::string data);
	void info(std::string data);
	void warn(std::string data);
	void error(std::string data);

	static Tabulator tabulator(bool add = true);

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

	// Log a message.
	void log(
		LoggerLevel messageLevel,
		const char* levelName,
		const std::string& data
	);
};

// Get a logger using the level configured for its key.
Logger getLogger(const std::string& key);