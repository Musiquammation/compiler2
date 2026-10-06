#pragma once

#include "../util/MemorySize.hpp"
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
