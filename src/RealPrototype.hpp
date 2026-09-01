#pragma once

#include "Prototype.hpp"
#include <map>

struct VariadicPrototype;
struct Class;

struct RealPrototype {
	int owners;
	Class* clas;
	int size;
	int align;
	std::map<VariadicPrototype*, Prototype> settings;	
};
