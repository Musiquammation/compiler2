#pragma once

#include "../label_t.hpp"
#include "symbol/ClassSymbol.hpp"
#include "../util/MemoryPlacement.hpp"
#include <memory>
#include <optional>
#include <variant>
#include <vector>

struct RealPrototype;
struct ProcResPrototype;
struct MemorySize;
struct ClassMember;

struct FollowClassMembers;

struct Prototype: std::variant<
	std::shared_ptr<RealPrototype>,
	std::shared_ptr<ProcResPrototype>
> {
	MemorySize evalQuickSize() const;
	ClassSymbol getMinimalClass() const;
	const Class* forceClass() const;
	std::optional<FollowClassMembers> followClassMembers(label_t* array, size_t length) const;

	bool isEntire() const;

	static Prototype& forceResolution(std::optional<Prototype>& proto);

	MemorySize evalProtoSize(std::vector<const RealPrototype*>& stack) const;

	std::optional<Prototype> getMeta() const;
	Prototype resolveMeta() const;
	bool isTriviallyNoMeta() const;
};




struct FollowClassMembers {
	std::vector<ClassMember*> members;
	Prototype proto;
	MemoryPlacement placement;
};