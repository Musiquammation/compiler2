#include "MemoryPlacer.hpp"
#include <algorithm>

int MemoryPlacer::add(MemorySize size) {
	// If the incoming size is invalid, invalidate the internal state and return -1.
	if (!size.isValid()) {
		this->size = -1;
		this->align = -1;
		this->offset = -1;
		return -1;
	}

	int alignment = std::max(size.align, 1);

	// Align the current offset.
	int padding = (alignment - (offset % alignment)) % alignment;

	offset += padding;
	int placement = offset;

	offset += size.size;

	// Update the total size and alignment.
	this->size = offset;
	this->align = std::max(this->align, alignment);

	return placement;
}

MemorySize MemoryPlacer::evalSize() const {
	// If the size is invalid, return MEMORY_SIZE_UNKNOWN.
	if (size < 0 || align < 0) {
		return MEMORY_SIZE_UNKNOWN;
	}

	// Align the final size to the maximum alignment.
	int padding = (align - (size % align)) % align;

	return MemorySize{
		.size = size + padding,
		.align = align
	};
}