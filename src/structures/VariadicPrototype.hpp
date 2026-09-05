#pragma once

#include "../label_t.hpp"
#include "Prototype.hpp"
#include "symbol/ClassSymbol.hpp"
#include <memory>
#include <optional>
#include <vector>

struct ClassMember;
struct ClassSymbol;

struct VariadicPrototype {
	ClassMember* reference;
	std::optional<Prototype> minimal;

	ClassSymbol getMinimalClass() const;
	std::vector<ClassMember*> followClassMembers(label_t* array, size_t length) const;

	static std::shared_ptr<VariadicPrototype> create(
		ClassMember* reference,
		Prototype minimal
	);

	static std::shared_ptr<VariadicPrototype> create(
		ClassMember* reference
	);
};
