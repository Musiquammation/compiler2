#pragma once

#include "Prototype.hpp"
#include "../util/MemorySize.hpp"
#include "symbol/ClassSymbol.hpp"
#include <map>
#include <memory>

struct ClassMember;


struct RealPrototype {
	ClassSymbol clas;
	MemorySize size;
	std::map<ClassMember*, Prototype> settings;

	static std::shared_ptr<RealPrototype> create(
		ClassSymbol clas,
		std::map<ClassMember*, Prototype> settings
	);


	ClassSymbol getMinimalClass() const;
	std::vector<ClassMember*> followClassMembers(label_t* array, size_t length) const;

private:
	RealPrototype() = default;
};
