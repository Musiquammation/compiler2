#include <format>
#include <variant>

#include "ClassSymbol.hpp"
#include "../StructureError.hpp"

class ClassSymbolResolutionError : StructureError {
public:
	ClassSymbolResolutionError(label_t symbol):
		StructureError(std::format("Cannot find {}", symbol)) {}
};

const Class* ClassSymbol::force() const {
	return std::visit([](const auto& symbol) {
		using T = std::decay_t<decltype(symbol)>;

		if constexpr (std::is_same_v<T, const Class*>) {
			return symbol;
		}

		if constexpr (std::is_same_v<T, label_t>) {
			throw ClassSymbolResolutionError{symbol};
		}

		return (const Class*)nullptr;
	}, *this);
}

template <bool force>
const Class* ClassSymbol::resolve() {
	return std::visit([](const auto& symbol) {
		using T = std::decay_t<decltype(symbol)>;

		if constexpr (std::is_same_v<T, const Class*>) {
			return symbol;
		}

		if constexpr (std::is_same_v<T, label_t>) {
			/// TODO: search symbol

			if constexpr (force) {
				throw ClassSymbolResolutionError{symbol};
			} else {
				return (const Class*)nullptr;
			}

		}

		return (const Class*)nullptr;
	}, *this);
}

const Class* ClassSymbol::get() const {
	return std::visit([](const auto& symbol) {
		using T = std::decay_t<decltype(symbol)>;

		if constexpr (std::is_same_v<T, const Class*>) {
			return symbol;
		} else {
			return (const Class*)nullptr;
		}
	}, *this);
}



template const Class* ClassSymbol::resolve<true>();
template const Class* ClassSymbol::resolve<false>();