#include "MemorySize.hpp"


bool MemorySize::isValid() const {
    return this->size >= 0 && this->align >= 0;
}
