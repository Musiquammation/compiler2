#pragma once

#include "label_t.hpp"

class ClassMember;

class Class {
	label_t name;
	Class* meta;
	ClassMember* classMember;
};

