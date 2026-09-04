#pragma once

#include "Prototype.hpp"
#include "../util/MemorySize.hpp"
#include "symbol/ClassSymbol.hpp"
#include <map>
#include <memory>

struct VariadicPrototype;


struct RealPrototype {
	ClassSymbol clas;
	MemorySize size;
	std::map<VariadicPrototype*, Prototype> settings;

	static std::shared_ptr<RealPrototype> create(
		ClassSymbol clas,
		std::map<VariadicPrototype*, Prototype> settings
	);


	ClassSymbol getMinimalClass() const;
	std::vector<ClassMember*> followClassMembers(label_t* array, size_t length) const;

private:
	RealPrototype() = default;
};
