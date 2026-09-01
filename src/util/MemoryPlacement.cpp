#include "MemoryPlacement.hpp"
#include "MemorySize.hpp"


bool MemoryPlacement::isValid() const {
    return this->offset >= 0 && this->size.isValid();
}

