#pragma once

#include <variant>

class RealPrototype;
class VariadicPrototype;

using Prototype = std::variant<RealPrototype*, VariadicPrototype*>;

