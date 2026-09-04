#pragma once

#include <exception>
#include <string>

class StructureError: public std::exception {
private:
	std::string message;

public:
	StructureError(const char* kind, const std::string& message);

	const char* what() const noexcept override;
};

StructureError createStructureError(
	const char* kind,
	const std::string& message
);