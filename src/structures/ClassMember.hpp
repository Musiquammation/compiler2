#pragma once

#include "BuildablePrototype.hpp"
#include "classMembers_t.hpp"

#include "ProcResPrototype.hpp"
#include "RealPrototype.hpp"

#include "../label_t.hpp"
#include "../util/MemoryPlacement.hpp"

struct Class;
struct Primitive;

struct ClassMember {
	label_t name;
	Prototype proto;
	MemoryPlacement placement;
};


struct MetaClassMember {
	ClassMember member;
};

struct ClassMemberSubBuilder {
	label_t name;
	Prototype proto;
	ClassMember* build(MemoryPlacement placement);
};

struct ClassMemberBuilder {
	label_t name;
	BuildablePrototype proto;

	ClassMemberSubBuilder subBuild(
		classMembers_ct members,
		classMembers_ct metaMembers,
		size_t membersLength,
		size_t metaMembersLength
	);
};
