#pragma once

#include "Prototype.hpp"
#include "../util/MemorySize.hpp"
#include <map>
#include <memory>

struct VariadicPrototype;
struct Class;


struct RealPrototype {
	const Class* clas;
	MemorySize size;
	std::map<VariadicPrototype*, Prototype> settings;

	static std::shared_ptr<RealPrototype> create(
		const Class* clas,
		std::map<VariadicPrototype*, Prototype> settings
	);

private:
	RealPrototype() = default;
};
