#pragma once

#include "../util/MemorySize.hpp"
#include "BuildablePrototype.hpp"
#include "classMembers_t.hpp"
#include "symbol/ClassSymbol.hpp"
#include <map>
#include <memory>
#include <optional>
#include <vector>

struct ClassMember;
struct Prototype;


struct RealPrototype {
	ClassSymbol clas;
	MemorySize size;
	std::map<const ClassMember*, Prototype> settings;

	static std::shared_ptr<RealPrototype> create(
		ClassSymbol clas,
		std::map<const ClassMember*, Prototype> settings
	);


	std::optional<Prototype> searchVariadic(const ClassMember* member) const;

	ClassSymbol getMinimalClass() const;
	std::vector<ClassMember*> followClassMembers(label_t* array, size_t length) const;

	std::optional<Prototype> getMeta() const;
	Prototype resolveMeta() const;
	bool isMetaResolutionDistinct() const;
	bool isTriviallyNoMeta() const;

private:
	RealPrototype() = default;
};

struct RealPrototypeBuilder {
	ClassSymbol clas;
	std::map<const ClassMember*, BuildablePrototype> settings;

	std::shared_ptr<RealPrototype> build(
		classMembers_ct members,
		classMembers_ct metaMembers,
		size_t membersLength,
		size_t metaMembersLength
	);
};