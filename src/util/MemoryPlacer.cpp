#include "MemoryPlacer.hpp"
#include <algorithm>

int MemoryPlacer::add(MemorySize size) {
	if (!size.isValid()) {
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
	// Align the final size to the maximum alignment.
	int padding = (align - (size % align)) % align;

	return MemorySize{
		.size = size + padding,
		.align = align
	};
}
