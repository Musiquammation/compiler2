#pragma once

#include <variant>
#include "../../label_t.hpp"

struct Class;

struct ClassSymbol: std::variant<Class*, label_t> {
	Class* force() const;

	template<bool force>
	Class* resolve();
};

