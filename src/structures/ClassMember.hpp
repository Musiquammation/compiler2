#pragma once

#include "Prototype.hpp"
#include "VariadicPrototype.hpp"

#include "../label_t.hpp"
#include "../util/MemoryPlacement.hpp"
#include <variant>

struct Class;

struct ClassMember {
	label_t name;
	Prototype proto;
	MemoryPlacement placement;
};

struct ClassMemberSubBuilder {
	label_t name;
	Prototype proto;
	ClassMember build(MemoryPlacement placement);
};

struct ClassMemberBuilder {
	label_t name;
	std::variant<Prototype, VariadicPrototypeBuilder> proto;

	std::optional<ClassMemberBuilder> getMeta() const;
	ClassMemberSubBuilder subBuild(const Class* source) const;
};
