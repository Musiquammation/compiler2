#pragma once

#include "Prototype.hpp"

#include "../label_t.hpp"
#include "../util/MemoryPlacement.hpp"

struct ClassMember {
	label_t name;
	Prototype proto;
	MemoryPlacement placement;
};

struct ClassMemberBuilder {
	label_t name;
	Prototype proto;

	ClassMember build(MemoryPlacement placement);
};