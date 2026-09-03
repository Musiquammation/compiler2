#include <exception>
#include <string>


class StructureError: std::exception {
private:
	std::string message;

public:
	StructureError(const std::string& message): message(message) {}

	const char* what() const noexcept override {
		return message.c_str();
	}
};


