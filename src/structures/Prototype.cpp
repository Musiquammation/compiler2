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
#include <format>
#include <map>
#include <optional>
#include <variant>

static auto logger = getLogger("prototype");

MemorySize Prototype::getQuickSize() const {
	return std::visit([](const auto& proto) {
		using T = std::decay_t<decltype(proto)>;

		if constexpr (std::is_same_v<T, std::shared_ptr<RealPrototype>>) {
			logger.debug("quick size of '%s' (%d %d)", proto->clas.getSymbol(), proto->size.size, proto->size.align);
			return proto->size;
		} else {
			logger.warn("quick size of '%s' failed", proto->reference->name);
			return MEMORY_SIZE_UNKNOWN;
		}
	}, *this);
}



static MemorySize evalProtoSize(
	const Prototype& proto,
	std::vector<const RealPrototype*>& stack
) {
	return std::visit([&stack](const auto& proto) {
		using T = std::decay_t<decltype(proto)>;

		if constexpr (std::is_same_v<T, std::shared_ptr<RealPrototype>>) {
			RealPrototype* rp = proto.get();
			if (rp->size.isValid()) {
				logger.debug("direct  size of '%s' is (%d %d)", rp->clas.getSymbol(), rp->size.size, rp->size.align);
				return rp->size;
			}

			auto cl = rp->clas.resolve<true>();

			MemoryPlacer placer;

			logger.debug("Evaluating size of '%s'...", rp->clas.getSymbol());
			
			{
				auto _ = logger.tabulator();
				stack.push_back(rp);
				for (const auto& member: cl->members) {
					logger.debug("with member '%s':", member.name);
					auto _ = logger.tabulator();
					placer.add(evalProtoSize(member.proto, stack));
				}
				stack.pop_back();
			}
			

			auto fsize = placer.evalSize();
			logger.debug("evalued size of '%s' is (%d %d)", rp->clas.getSymbol(), fsize.size, fsize.align);
			return fsize;

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
						logger.debug(
							"Replacing variadic('%s') by real('%s')",
							member->name,
							foundProto.getMinimalClass().getSymbol()
						);

						auto _ = logger.tabulator();
						return {evalProtoSize(foundProto, stack)};
					} else if constexpr (std::is_same_v<T, std::shared_ptr<VariadicPrototype>>) {
						logger.debug(
							"Replacing variadic('%s') by variadic('%s')",
							member->name,
							p->reference->name
						);
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

MemorySize Prototype::evalSize() const {
	std::vector<const RealPrototype*> stack{};
	return evalProtoSize(*this, stack);
}

ClassSymbol Prototype::getMinimalClass() const {
	return std::visit([](const auto& proto) {
		return proto->getMinimalClass();
	}, *this);
}

std::optional<FollowClassMembers> Prototype::followClassMembers(label_t* array, size_t length) const {
	logger.info("followClassMembers inside '%s'", this->getMinimalClass().getSymbol());
	auto tabulator = Logger::tabulator();
	tabulator.add();

	if (length == 0)
		return std::nullopt;

	auto currentMember = this->getMinimalClass().force()->members.search(array[0]);
	
	if (currentMember == nullptr)
		return std::nullopt;

	std::vector<ClassMember*> members;
	members.push_back(currentMember);

	Prototype currentProto = currentMember->proto;
	
	std::map<const ClassMember*, Prototype> rawSettings;
	const std::map<const ClassMember*, Prototype>* settings = std::visit(
		[&rawSettings](const auto& proto) -> const std::map<const ClassMember*, Prototype>* {
			using T = std::decay_t<decltype(proto)>;

			if constexpr (std::is_same_v<T, std::shared_ptr<RealPrototype>>) {
				return &proto->settings;

			} else {
				return &rawSettings; // empty settings
			}
		},
		*this
	);

	label_t* const lastLabel = &array[length];
	label_t* label = &array[1];
	while (true) {
		logger.debug("Extracting settings of '%s'", currentProto.getMinimalClass().getSymbol());
		tabulator.add();
		for (const auto& [key, value]: *settings) {
			logger.debug("[%s=%s]", key->name, value.getMinimalClass().getSymbol());
		}
		tabulator.remove();


		// Take settings
		{
			std::map<const ClassMember*, Prototype> next;
			std::visit([&next, settings, &currentProto](const auto& proto) {
				using T = std::decay_t<decltype(proto)>;
	
				if constexpr (std::is_same_v<T, std::shared_ptr<RealPrototype>>) {
					RealPrototype* rp = proto.get();
					for (const auto& [key, value]: rp->settings) {
						bool add = std::visit([&next, &key, settings](const auto& value) {
							using U = std::decay_t<decltype(value)>;
	
							if constexpr (std::is_same_v<U, std::shared_ptr<VariadicPrototype>>) {
								auto it = settings->find(value->reference);
								if (it != settings->end()) {
									next.insert({key, it->second});
								} else {
									throw StructureError{
										"VariadicNotResolved",
										std::format("Cannot resolve variadic {}", value->reference->name)
									};
								}
								return false;
							}
	
							return true;
						}, value);
	
						if (add) {
							next.insert({key, value});
						}
					}

				} else {
					VariadicPrototype* vp = proto.get();
					auto it = settings->find(vp->reference);
					if (it != settings->end()) {
						logger.debug("Resolve '%s' in '%s'", vp->reference->name, it->second.getMinimalClass().getSymbol());
						currentProto = it->second;
					} else {
						throw StructureError{
							"VariadicNotResolved",
							std::format("Cannot resolve variadic {}", vp->reference->name)
						};
					}
				}
			}, currentProto);
	
			rawSettings = std::move(next);
			settings = &rawSettings;
		}

		if (label >= lastLabel)
			break;

		auto cl = currentProto.getMinimalClass();
		auto next = cl.force()->members.search(*label);
		if (next == nullptr)
			break;
		
		logger.debug("got member '%s' of '%s'", *label, currentMember->proto.getMinimalClass().getSymbol());
		label++;
		currentMember = next;
		currentProto = currentMember->proto;
		members.push_back(currentMember);
	}


	return {{
		.members = members,
		.proto = currentProto,
		.placement = {}
	}};
}

bool Prototype::isEntire() const {
	return std::visit([](const auto& proto) -> bool {
		using T = std::decay_t<decltype(proto)>;

		if constexpr (std::is_same_v<T, std::shared_ptr<RealPrototype>>) {
			return proto->clas.force()->isEntire();
		} else {
			return false;
		}

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
