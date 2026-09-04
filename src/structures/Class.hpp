#pragma once

#include "ClassMember.hpp"
#include "Primitive.hpp"
#include "../label_t.hpp"
#include "../util/MemorySize.hpp"
#include "../util/SortedArray.hpp"
#include <cstddef>
#include <memory>
#include <vector>

struct ClassMember;
struct Primitive;

struct Class {
	label_t name;
	std::unique_ptr<Class> meta;
	SortedArray<ClassMember, label_t, LabeledComparator<ClassMember>> members;
	const Primitive* primitive;
	MemorySize size;

	Class(
		label_t name,
		std::unique_ptr<Class> meta,
		std::unique_ptr<ClassMember[]>& classMembers,
		size_t classMemberLength,
		const Primitive* primitive,
		MemorySize size
	) :
		name{name},
		meta{std::move(meta)},
		members{classMembers, classMemberLength},
		primitive{primitive},
		size{size}
	{}

	Class(
		label_t name,
		const Primitive* primitive,
		MemorySize size
	);

	Class(){}

	static Class create(
		label_t name,
		std::unique_ptr<Class> meta,
		std::vector<ClassMemberBuilder>& members
	);

	static Class createPrimitive(const Primitive* p, label_t name);


	Class(const Class&) = delete;
	Class& operator=(const Class&) = delete;

	Class(Class&&) noexcept = default;
	Class& operator=(Class&&) noexcept = default;

	~Class() = default;
};
