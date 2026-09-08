#pragma once

#include "../label_t.hpp"
#include "symbol/ClassSymbol.hpp"
#include "../util/MemoryPlacement.hpp"
#include <memory>
#include <optional>
#include <variant>
#include <vector>

struct RealPrototype;
struct VariadicPrototype;
struct MemorySize;
struct ClassMember;

struct FollowClassMembers;

struct Prototype: std::variant<
	std::shared_ptr<RealPrototype>,
	std::shared_ptr<VariadicPrototype>
> {
	MemorySize getQuickSize() const;
	MemorySize resolveSize() const;
	ClassSymbol getMinimalClass() const;
	std::optional<FollowClassMembers> followClassMembers(label_t* array, size_t length) const;

	template<typename Resolver>
	Prototype resolve(const Resolver& resolver);

	static Prototype& forceResolution(std::optional<Prototype>& proto);
};




struct FollowClassMembers {
	std::vector<ClassMember*> members;
	Prototype proto;
	MemoryPlacement placement;
};