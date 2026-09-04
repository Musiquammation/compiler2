#pragma once

#include "../label_t.hpp"
#include <vector>

struct ClassMember;
struct ClassSymbol;

struct VariadicPrototype {
	int owners;
	ClassMember* member;

	ClassSymbol getMinimalClass() const;
	std::vector<ClassMember*> followClassMembers(label_t* array, size_t length) const;
};
