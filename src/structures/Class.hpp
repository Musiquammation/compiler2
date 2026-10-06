#pragma once

#include "ClassMember.hpp"
#include "Primitive.hpp"
#include "../label_t.hpp"
#include "../util/MemorySize.hpp"
#include "../util/TracedSortedArray.hpp"
#include <cstddef>
#include <memory>
#include <vector>

struct ClassMember;
struct Primitive;

struct Class {
	label_t name;
	std::unique_ptr<Class> meta;
	TracedSortedArray<ClassMember, label_t, LabeledComparator<ClassMember>> members;
	const Primitive* primitive;
	MemorySize size;

	friend struct ClassBuilder;

private:
	bool _isEntire;

	Class(
		label_t name,
		std::unique_ptr<Class> meta,
		std::unique_ptr<ClassMember[]>& classMembers,
		size_t classMemberLength,
		const Primitive* primitive,
		MemorySize size,
		bool isEntire
	):
		name{name},
		meta{std::move(meta)},
		members{classMembers, classMemberLength},
		primitive{primitive},
		size{size},
		_isEntire{isEntire}
	{}

public:
	Class(
		label_t name,
		const Primitive* primitive,
		MemorySize size
	);

	Class(){}

	bool isEntire() const {
		return this->_isEntire;
	}

	static Class createPrimitive(const Primitive* p, label_t name);


	Class(const Class&) = delete;
	Class& operator=(const Class&) = delete;

	Class(Class&&) noexcept = default;
	Class& operator=(Class&&) noexcept = default;

	~Class() = default;
};

struct ClassBuilder {
	label_t name;
	std::vector<ClassMemberBuilder> members;
	ClassBuilder* meta;

	std::unique_ptr<Class> build();


	static std::unique_ptr<Class> generateClass(
		label_t name,
		std::unique_ptr<Class> meta,
		std::unique_ptr<ClassMember[]>& classMembers,
		size_t classMemberLength,
		const Primitive* primitive,
		MemorySize size,
		bool isEntire
	);
};


