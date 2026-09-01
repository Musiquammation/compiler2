#pragma once

struct MemorySize {
	int size;
	int align;

	bool isValid() const;

};

constexpr MemorySize MEMORY_SIZE_UNKNOWN = {-1, -1};
