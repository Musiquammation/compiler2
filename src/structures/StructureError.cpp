#include "StructureError.hpp"

#include <iostream>

StructureError::StructureError(
	const char* kind,
	const std::string& message
):
	message{std::string{"["} + kind + "] " + message},
	trace{std::stacktrace::current()}
{}

const char* StructureError::what() const noexcept {
	return message.c_str();
}

void StructureError::printBacktrace() const {
	std::cerr << "Backtrace:\n";

	for (const auto& frame : trace) {
		std::cerr << "    " << frame.description();

		if (!frame.source_file().empty()) {
			std::cerr
				<< " at "
				<< frame.source_file()
				<< ':'
				<< frame.source_line();
		}

		std::cerr << '\n';
	}
}

StructureError createStructureError(
	const char* kind,
	const std::string& message
) {
	return StructureError{kind, message};
}