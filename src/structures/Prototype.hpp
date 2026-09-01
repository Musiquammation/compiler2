#pragma once

#include <memory>
#include <variant>

struct RealPrototype;
struct VariadicPrototype;

using Prototype = std::variant<std::shared_ptr<RealPrototype>, std::shared_ptr<VariadicPrototype>>;

