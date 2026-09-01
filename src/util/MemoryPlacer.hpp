#pragma once

#include "MemorySize.hpp"


class MemoryPlacer {
	int size = 0;
	int align = 0;
	int offset = 0;

public:
	int add(MemorySize size);
	MemorySize evalSize() const;
};
