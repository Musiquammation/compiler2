#pragma once

#include "Class.hpp"
#include "Prototype.hpp"
#include "Type.hpp"

class Variable {
    label_t name;
    Prototype proto;
    Type* type;
};
