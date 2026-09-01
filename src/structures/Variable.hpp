#pragma once

#include "Class.hpp"
#include "Prototype.hpp"
#include "Type.hpp"

struct Variable {
    label_t name;
    Prototype proto;
    Type* type;
};
