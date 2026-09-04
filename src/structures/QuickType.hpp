#pragma once

#include "Prototype.hpp"
#include "Type.hpp"
#include <memory>
#include <variant>


struct QuickType: std::variant<std::shared_ptr<Type>, Prototype> {
	Prototype getPrototype();
};

