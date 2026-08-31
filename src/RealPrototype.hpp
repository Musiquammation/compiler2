#pragma once

#include "Prototype.hpp"
#include <map>

class VariadicPrototype;
class Class;

class RealPrototype {
	int owners;
	Class* clas;
	int size;
	int align;
	std::map<VariadicPrototype*, Prototype> settings;
	
	
	RealPrototype(Class* clas);
};
