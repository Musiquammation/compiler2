#include "Class.hpp"
#include "ClassMember.hpp"

Class Class::createPrimitive(const Primitive* p, label_t name) {
	return Class{
		.name = name,
		.meta = nullptr,
		.classMembers = nullptr,
		.primitive = p,
		.classMemberLength = 0
	};
}

Class::~Class() {
	if (this->classMembers) {
		delete[] this->classMembers;
	}
}

