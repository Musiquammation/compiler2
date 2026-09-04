#include "StructureError.hpp"

#include <format>

StructureError::StructureError(
	const char* kind,
	const std::string& message
):
	message(std::format("[{}] {}", kind, message))
{}

const char* StructureError::what() const noexcept {
	return message.c_str();
}

StructureError createStructureError(
	const char* kind,
	const std::string& message
) {
	return StructureError(kind, message);
}
