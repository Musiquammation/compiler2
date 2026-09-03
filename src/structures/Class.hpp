#pragma once

#include "ClassMember.hpp"
#include "Primitive.hpp"
#include "../label_t.hpp"
#include "../util/MemorySize.hpp"
#include <cstddef>
#include <memory>
#include <vector>

struct ClassMember;
struct Primitive;

struct Class {
	label_t name;
	std::unique_ptr<Class> meta;
	std::unique_ptr<ClassMember[]> classMembers;
	const Primitive* primitive;
	size_t classMemberLength;
	MemorySize size;

	Class(){}

	Class(
		label_t name,
		std::unique_ptr<Class> meta,
		std::unique_ptr<ClassMember[]> classMembers,
		const Primitive* primitive,
		size_t classMemberLength,
		MemorySize size
	) :
		name{name},
		meta{std::move(meta)},
		classMembers{std::move(classMembers)},
		primitive{primitive},
		classMemberLength{classMemberLength},
		size{size}
	{}

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
