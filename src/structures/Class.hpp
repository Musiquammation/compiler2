#pragma once

#include "Primitive.hpp"
#include "../label_t.hpp"

struct ClassMember;
struct Primitive;

struct Class {
	label_t name;
	Class* meta;
	ClassMember* classMembers;
	const Primitive* primitive;
	int classMemberLength;


	static Class createPrimitive(const Primitive* p, label_t name);

	~Class();
};

