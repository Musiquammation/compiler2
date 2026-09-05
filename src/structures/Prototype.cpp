#include "Prototype.hpp"
#include "ClassMember.hpp"
#include "RealPrototype.hpp"
#include "VariadicPrototype.hpp"
#include "StructureError.hpp"
#include "VariadicPrototype.hpp"
#include "Class.hpp"
#include "../util/Logger.hpp"
#include "../util/MemorySize.hpp"
#include "../util/MemoryPlacer.hpp"
#include <optional>

static auto logger = getLogger("prototype");

MemorySize Prototype::getQuickSize() const {
	return std::visit([](const auto& proto) {
		using T = std::decay_t<decltype(proto)>;

		if constexpr (std::is_same_v<T, std::shared_ptr<RealPrototype>>) {
			logger.debug("qkc_size of %s %d %d", proto->clas.getSymbol(), proto->size.size, proto->size.align);
			return proto->size;
		} else {
			return MEMORY_SIZE_UNKNOWN;
		}
	}, *this);
}



static MemorySize resolveProtoSize(
	const Prototype& proto,
	std::vector<const RealPrototype*>& stack
) {
	return std::visit([&stack](const auto& proto) {
		using T = std::decay_t<decltype(proto)>;

		if constexpr (std::is_same_v<T, std::shared_ptr<RealPrototype>>) {
			const std::shared_ptr<RealPrototype>& rp = proto;
			logger.debug("rsv_size of %s %d %d", rp->clas.getSymbol(), rp->size.size, rp->size.align);
			if (rp->size.isValid())
				return rp->size;

			auto cl = rp->clas.resolve<true>();

			MemoryPlacer placer;
			
			stack.push_back(rp.get());
			for (const auto& member: cl->members) {
				placer.add(resolveProtoSize(member.proto, stack));
			}
			stack.pop_back();

			return placer.evalSize();

		} else {
			const ClassMember* member = proto->reference;
			for (auto it = stack.rbegin(); it != stack.rend(); ++it) {
				auto rp = *it;
				auto foundIt = rp->settings.find(member);
				if (foundIt == rp->settings.end()) {
					return MEMORY_SIZE_UNKNOWN; // not found
				}

				auto& foundProto = foundIt->second;

				auto result = std::visit([&member, &foundProto](const auto& p) -> std::optional<MemorySize> {
					using T = std::decay_t<decltype(p)>;

					if constexpr (std::is_same_v<T, std::shared_ptr<RealPrototype>>) {
						std::vector<const RealPrototype*> stack{};
						return {resolveProtoSize(foundProto, stack)};
					} else if constexpr (std::is_same_v<T, std::shared_ptr<VariadicPrototype>>) {
						member = p->reference;
						return std::nullopt;
					}
				}, foundProto);

				if (result.has_value()) {
					return result.value();
				}

			}

			return MEMORY_SIZE_UNKNOWN;
		}
	}, proto);
}

MemorySize Prototype::resolveSize() const {
	std::vector<const RealPrototype*> stack{};
	return resolveProtoSize(*this, stack);
}

ClassSymbol Prototype::getMinimalClass() const {
	return std::visit([](const auto& proto) {
		return proto->getMinimalClass();
	}, *this);
}

std::vector<ClassMember*> Prototype::followClassMembers(label_t* array, size_t length) const {
	return std::visit([array, length](const auto& proto) {
		return proto->followClassMembers(array, length);
	}, *this);
}


template<typename Resolver>
Prototype Prototype::resolve(const Resolver& resolver) {
	return std::visit([&resolver](const auto& proto) -> Prototype {
		using T = std::decay_t<decltype(proto)>;

		if constexpr (std::is_same_v<T, std::shared_ptr<RealPrototype>>) {
			return {proto};

		} else {
			std::optional<Prototype> def = resolver.searchVariadic(proto->reference);
			if (def.has_value())
				return {def.value()};
			
			return {proto};
		}


	}, *this);
}



Prototype& Prototype::forceResolution(std::optional<Prototype>& proto) {
	if (!proto) {
		throw StructureError{
			"VariadicResolutionError", 
			"Could not resolve VariadicPrototype"
		};
	}

	return *proto;
}




template Prototype Prototype::resolve(const RealPrototype& resolver);
