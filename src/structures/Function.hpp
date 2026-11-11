#pragma once

#include "../label_t.hpp"
#include "Prototype.hpp"
#include <string>
#include <vector>

struct Argument {
	label_t name;
	Prototype proto;
	MemoryPlacement placement;
	size_t varId = 0xffffffffffffffff;
};

struct ReturnType {
	struct Void {};
	struct Auto {};

	std::variant<Prototype, Void, Auto> proto;
};

struct Function {
	std::vector<Argument> args;

	ReturnType ret;

	std::string cname;
};
