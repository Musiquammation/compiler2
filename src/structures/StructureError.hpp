#pragma once

#include <exception>
#include <stacktrace>
#include <string>

class StructureError: public std::exception {
private:
	std::string message;
	std::stacktrace trace;

public:
	StructureError(const char* kind, const std::string& message);

	const char* what() const noexcept override;

	void printBacktrace() const;
};

StructureError createStructureError(
	const char* kind,
	const std::string& message
);