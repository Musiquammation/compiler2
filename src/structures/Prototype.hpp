#pragma once

#include "../label_t.hpp"
#include "symbol/ClassSymbol.hpp"
#include <memory>
#include <variant>
#include <vector>

struct RealPrototype;
struct VariadicPrototype;
struct MemorySize;
struct ClassMember;

struct Prototype: std::variant<
	std::shared_ptr<RealPrototype>,
	std::shared_ptr<VariadicPrototype>
> {
	
	MemorySize getQuickSize() const;
	ClassSymbol getMinimalClass() const;
	std::vector<ClassMember*> followClassMembers(label_t* array, size_t length) const;

};


