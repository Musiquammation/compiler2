#include <format>
#include <variant>

#include "ClassSymbol.hpp"
#include "../StructureError.hpp"

class ClassSymbolResolutionError : StructureError {
public:
	ClassSymbolResolutionError(label_t symbol):
		StructureError(std::format("Cannot find {}", symbol)) {}
};

Class* ClassSymbol::force() const {
	return std::visit([](const auto& symbol) {
		using T = std::decay_t<decltype(symbol)>;

		if constexpr (std::is_same_v<T, Class*>) {
			return symbol;
		}

		if constexpr (std::is_same_v<T, label_t>) {
			throw ClassSymbolResolutionError{symbol};
		}

		return (Class*)nullptr;
	}, *this);
}

template <bool force>
Class* ClassSymbol::resolve() {
	return std::visit([](const auto& symbol) {
		using T = std::decay_t<decltype(symbol)>;

		if constexpr (std::is_same_v<T, Class*>) {
			return symbol;
		}

		if constexpr (std::is_same_v<T, label_t>) {
			/// TODO: search symbol

			if constexpr (force) {
				throw ClassSymbolResolutionError{symbol};
			} else {
				return (Class*)nullptr;
			}

		}

		return (Class*)nullptr;
	}, *this);
}


template Class* ClassSymbol::resolve<true>();
template Class* ClassSymbol::resolve<false>();