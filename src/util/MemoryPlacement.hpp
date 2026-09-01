#pragma once

#include "MemorySize.hpp"
struct MemoryPlacement {
	MemorySize size;
    int offset;

	bool isValid() const;
};
