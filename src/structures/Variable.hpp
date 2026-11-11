#pragma once

#include "Prototype.hpp"
#include "../util/MemoryPlacement.hpp"
#include <cstddef>


class Variable {
public:
	struct PositionNode;

private:
	PositionNode* position = nullptr;

public:
	label_t name;
	Prototype proto;

	Variable(label_t name, Prototype proto):
		name(name),
		proto(proto)
	{}

	~Variable();

	void resolvePlacements();
	MemoryPlacement getPlacement(label_t* array, size_t length);
	MemorySize getSize();
};
