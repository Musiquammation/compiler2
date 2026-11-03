#pragma once
#include "classMembers_t.hpp"
#include <cstddef>
#include <variant>

struct Prototype;
struct ProcResPrototypeBuilder;
struct RealPrototypeBuilder;

using BuildablePrototype = std::variant<
	Prototype,
	ProcResPrototypeBuilder,
	RealPrototypeBuilder
>;

Prototype buildBuildableProto(
	BuildablePrototype& builder,
	classMembers_ct members,
	classMembers_ct metaMembers,
	size_t membersLength,
	size_t metaMembersLength

);


