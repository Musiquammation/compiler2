#pragma once

#include <variant>

struct RealPrototype;
struct VariadicPrototype;

using Prototype = std::variant<RealPrototype*, VariadicPrototype*>;

