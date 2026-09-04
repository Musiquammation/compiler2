#pragma once

#include <variant>
#include "../../label_t.hpp"

struct Class;

struct ClassSymbol: std::variant<const Class*, label_t> {
	static const ClassSymbol NONE;

	/**
	 * Throws an error if symbol is not a Class
	 */
	const Class* force() const;

	/**
	 * Tries to resolve symbol,
	 * and throws an error if force is enabled
	 * 
	 * @return Class* or nullptr
	 */
	template<bool force>
	const Class* resolve();

	/**
	 * @return Class* or nullptr
	 */
	const Class* get() const;

	/**
	 * @return true if symbol is NULL
	 */
	bool isNull() const;
};

