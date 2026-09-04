#include "QuickType.hpp"
#include "Prototype.hpp"

#include <variant>

Prototype QuickType::getPrototype() {
	return std::visit([](const auto& qtype) {
		using T = std::decay_t<decltype(qtype)>;

		if constexpr (std::is_same_v<T, Prototype>) {
			return qtype;
		} else {
			return qtype->proto;
		}
	}, *this);
}


