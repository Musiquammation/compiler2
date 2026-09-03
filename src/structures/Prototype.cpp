#include "Prototype.hpp"
#include "RealPrototype.hpp"
#include "VariadicPrototype.hpp"
#include "../util/Logger.hpp"
#include "../util/MemorySize.hpp"

static auto logger = getLogger("prototype");

MemorySize Prototype::getQuickSize() const {
	return std::visit([](const auto& proto) {
		using T = std::decay_t<decltype(proto)>;

		if constexpr (std::is_same_v<T, std::shared_ptr<RealPrototype>>) {
			logger.debug("got size %d %d", proto->size.size, proto->size.align);
			return proto->size;
		}
		
		if constexpr (std::is_same_v<T, std::shared_ptr<VariadicPrototype>>) {
			return MEMORY_SIZE_UNKNOWN;
		}

		return MEMORY_SIZE_UNKNOWN;;
	}, *this);
}

