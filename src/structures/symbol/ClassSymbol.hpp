#pragma once

#include <variant>
#include "../../label_t.hpp"

struct Class;

struct ClassSymbol: std::variant<const Class*, label_t> {
	const Class* force() const;

	template<bool force>
	const Class* resolve();
};

