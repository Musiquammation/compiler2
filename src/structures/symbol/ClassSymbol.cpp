#include <format>
#include <variant>

#include "ClassSymbol.hpp"
#include "../StructureError.hpp"

const ClassSymbol ClassSymbol::NONE = ClassSymbol((label_t)nullptr);

static StructureError forceFailedError(label_t symbol) {
	if (symbol == nullptr) {
		return StructureError{
			"ClassSymbolResolutionError",
			std::format("Tried to resolve NULL symbol")
		};
	}

	return StructureError{
		"ClassSymbolResolutionError",
		std::format("Symbol {} was not resolved", symbol)
	};
}

const Class* ClassSymbol::force() const {
	return std::visit([](const auto& symbol) {
		using T = std::decay_t<decltype(symbol)>;

		if constexpr (std::is_same_v<T, const Class*>) {
			return symbol;
		} else {
			throw forceFailedError(symbol);
			return (const Class*)nullptr; // unreachable
		}

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
				throw forceFailedError(symbol);
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




bool ClassSymbol::isNull() const {
	return std::holds_alternative<label_t>(*this)
		&& std::get<label_t>(*this) == nullptr;
}


template const Class* ClassSymbol::resolve<true>();
template const Class* ClassSymbol::resolve<false>();