#pragma once

#include "Prototype.hpp"
#include "../util/MemoryPlacement.hpp"

struct Type;

class Variable {
public:
	struct PositionNode;

private:
	PositionNode* position = nullptr;

public:
	label_t name;
	Prototype proto;
	Type* type;

	Variable(label_t name, Prototype proto, Type* type):
		name(name),
		proto(proto),
		type(type)
	{}

	~Variable();

	void resolvePlacements();
	MemoryPlacement getPlacement(label_t* array, size_t length);
	int getSize() const;
};
