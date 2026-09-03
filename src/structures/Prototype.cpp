#include "Prototype.hpp"
#include "RealPrototype.hpp"
#include "VariadicPrototype.hpp"
#include "../util/Logger.hpp"
#include "../util/MemorySize.hpp"

static auto logger = getLogger("prototype");

MemorySize Prototype::getQuickSize() const {
	MemorySize size;

	std::visit([&size](const auto& proto) {
		using T = std::decay_t<decltype(proto)>;

		if constexpr (std::is_same_v<T, std::shared_ptr<RealPrototype>>) {
			logger.debug("got size %d %d", size.size, size.align);
			size = proto->size;
		} else if constexpr (std::is_same_v<T, std::shared_ptr<VariadicPrototype>>) {
			size = MEMORY_SIZE_UNKNOWN;
		}
	}, *this);

	return size;
}
