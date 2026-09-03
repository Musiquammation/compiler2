#pragma once

#include <memory>
#include <variant>

struct RealPrototype;
struct VariadicPrototype;
struct MemorySize;

struct Prototype: std::variant<
	std::shared_ptr<RealPrototype>,
	std::shared_ptr<VariadicPrototype>
> {
	MemorySize getQuickSize() const;
};

