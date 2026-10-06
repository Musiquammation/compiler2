#include "Prototype.hpp"
#include "ProcResPrototype.hpp"
#include "RealPrototype.hpp"
#include "ClassMember.hpp"
#include "StructureError.hpp"
#include "Class.hpp"
#include "../util/Logger.hpp"
#include "../util/MemorySize.hpp"
#include "../util/MemoryPlacer.hpp"
#include <cstddef>
#include <format>
#include <map>
#include <optional>
#include <variant>

static auto logger = getLogger("prototype");




MemorySize Prototype::evalProtoSize(std::vector<const RealPrototype*>& stack) const {
	return std::visit([&stack](const auto& proto) -> MemorySize {
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
					placer.add(member.proto.evalProtoSize(stack));
				}
				stack.pop_back();
			}
			

			auto fsize = placer.evalSize();
			logger.debug("evalued size of '%s' is (%d %d)", rp->clas.getSymbol(), fsize.size, fsize.align);
			return fsize;

		} else {
			auto result = proto->evalProtoSize(stack);
			return result.has_value() ? result.value() : MEMORY_SIZE_UNKNOWN;
		}
	}, *this);
}

std::optional<Prototype> Prototype::getMeta() const {
	return std::visit([](auto& p) {
		return p->getMeta();
	}, *this);
}

Prototype Prototype::resolveMeta() const {
	return std::visit([](auto& p) {
		if (p->isMetaResolutionDistinct()) {
			return p->resolveMeta();
		}

		return Prototype{p};
	}, *this);
}

bool Prototype::isTriviallyNoMeta() const {
	return std::visit([](auto& p) {
		return p->isTriviallyNoMeta();
	}, *this);
}


MemorySize Prototype::evalQuickSize() const {
	std::vector<const RealPrototype*> stack{};
	return this->evalProtoSize(stack);
}

ClassSymbol Prototype::getMinimalClass() const {
	return std::visit([](const auto& proto) {
		return proto->getMinimalClass();
	}, *this);
}

const Class* Prototype::forceClass() const {
	return std::visit([](const auto& proto) -> const Class* {
		using T = std::decay_t<decltype(proto)>;

		if constexpr (std::is_same_v<T, std::shared_ptr<RealPrototype>>) {
			return  proto->clas.force();
		}

		return nullptr;
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
						bool add = std::visit([&next, &key, settings](const auto& value) -> bool {
							using U = std::decay_t<decltype(value)>;
	
							if constexpr (std::is_same_v<U, std::shared_ptr<ProcResPrototype>>) {
								std::optional<Prototype> r = value->search(settings);
								if (r.has_value()) {
									next.insert({key, r.value()});
								} else {
									throw StructureError{
										"VariadicNotResolved",
										std::format("Cannot resolve variadic '{}'", value->getSymbolPlaceholder())
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
					// ProcResPrototype* vp = proto.get();
					throw StructureError{
						"TODO",
						"resolve fake vp"
					};
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






Prototype& Prototype::forceResolution(std::optional<Prototype>& proto) {
	if (!proto) {
		throw StructureError{
			"VariadicResolutionError", 
			"Could not resolve VariadicPrototype"
		};
	}

	return *proto;
}




