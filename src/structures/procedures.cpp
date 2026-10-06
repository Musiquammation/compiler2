#include "procedures.hpp"

#include "ClassMember.hpp"
#include "Procedure.hpp"
#include "Prototype.hpp"
#include "StructureError.hpp"
#include "symbol/ClassSymbol.hpp"

#include "../util/Logger.hpp"
#include "../poolBase.hpp"

#include <cstddef>
#include <format>
#include <optional>
#include <unordered_map>


using Entries = Procedure::Entries;
using Stack = std::vector<const RealPrototype*>;

template<typename T>
const T& Procedure::Entries::get(int idx) const {
	if (idx < 0 || static_cast<size_t>(idx) >= this->entries.size()) {
		throw StructureError{
			"InvalidEntry",
			std::format(
				"Entry index {} is out of bounds (size: {})",
				idx,
				this->entries.size()
			)
		};
	}

	const ProcedureEntry& entry = this->entries[idx];

	if (const T* value = std::get_if<T>(&entry)) {
		return *value;
	}

	throw StructureError{
		"InvalidEntry",
		std::format(
			"Expected {} but got {}",
			typeid(T).name(),
			std::visit([]<typename U>(const U&) {
				return std::string(typeid(U).name());
			}, entry)
		)
	};
}

namespace variadic {
	static auto logger = getLogger("procedures-variadic");

	static constexpr int args = 1;

	std::optional<MemorySize> evalProtoSize(Entries entries, Stack& stack) {
		const ClassMember* member = entries.get<PEMetaClassMember>(0).member;
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
					return {foundProto.evalProtoSize(stack)};
				} else {
					auto reference = p->getReferencedMember();
					logger.debug(
						"Replacing variadic('%s') by variadic('%s')",
						member->name,
						reference->name
					);
					member = reference;
					return std::nullopt;
				}
			}, foundProto);

			if (result.has_value()) {
				return result.value();
			}
		}

		return MEMORY_SIZE_UNKNOWN;
	}
	
	ClassSymbol minimalClass(Entries entries) {
		return ClassSymbol::NONE;
	}
	
	label_t getSymbolPlaceholder(Entries entries) {
		return entries.get<PEMetaClassMember>(0).member->name;
	}

	const ClassMember* getReferencedMember(Entries entries) {
		throw StructureError{
			"TODO",
			"getReferencedMember"
		};
	}

	std::optional<Prototype> search(
		Entries entries,
		const std::map<const ClassMember *, Prototype>* settings
	) {
		auto member = entries.get<PEMetaClassMember>(0).member;
		auto it = settings->find(member);
		if (it == settings->end())
			return std::nullopt;

		return it->second;
	}

	std::optional<Prototype> getMeta(Entries entries) {
		throw StructureError{
			"ProcedureUndefined",
			"Cannot call getMeta on variadic"
		};
	}

	Prototype resolveMeta(Entries entries) {
		auto mb = entries.get<PEMetaClassMember>(0);
		auto fv = getProcedure(poolBase.friendVariadic);

		return Prototype{
			std::make_shared<ProcResPrototype>(
				ProcResPrototype{
					.proc = {
						.procedure = fv,
						.entries = {
							PEClassMember{mb.member}
						}
					}
				}
			)
		};
	}
}


namespace friendVariadic {
	static auto logger = getLogger("procedures-friendVariadic");

	static constexpr int args = 1;

	std::optional<MemorySize> evalProtoSize(Entries entries, Stack& stack) {
		const ClassMember* member = entries.get<PEClassMember>(0).member;
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
					return {foundProto.evalProtoSize(stack)};
				} else {
					auto reference = p->getReferencedMember();
					logger.debug(
						"Replacing variadic('%s') by variadic('%s')",
						member->name,
						reference->name
					);
					member = reference;
					return std::nullopt;
				}
			}, foundProto);

			if (result.has_value()) {
				return result.value();
			}
		}

		return MEMORY_SIZE_UNKNOWN;
	}
	
	ClassSymbol minimalClass(Entries entries) {
		return ClassSymbol::NONE;
	}
	
	label_t getSymbolPlaceholder(Entries entries) {
		return entries.get<PEClassMember>(0).member->name;
	}

	const ClassMember* getReferencedMember(Entries entries) {
		throw StructureError{
			"TODO",
			"friendVariadic::getReferencedMember"
		};
	}

	std::optional<Prototype> search(
		Entries entries,
		const std::map<const ClassMember *, Prototype>* settings
	) {
		auto member = entries.get<PEClassMember>(0).member;
		auto it = settings->find(member);
		if (it == settings->end())
			return std::nullopt;

		return it->second;
	}

	std::optional<Prototype> getMeta(Entries entries) {
		logger.warn("Returning nullopt on friendVariadic.getMeta");
		return std::nullopt;

		throw StructureError{
			"ProcedureUndefined",
			"Cannot call resolveMeta on friendVariadic"
		};
	}

	Prototype resolveMeta(Entries entries) {
		throw StructureError{
			"ProcedureUndefined",
			"Cannot call resolveMeta on friendVariadic"
		};
	}
}




#define FILL(proc) { \
	poolBase.proc, \
	Procedure{ \
		.args = proc::args, \
		.evalProtoSize = &proc::evalProtoSize, \
		.minimalClass = &proc::minimalClass, \
		.getSymbolPlaceholder = &proc::getSymbolPlaceholder, \
		.getReferencedMember = &proc::getReferencedMember, \
		.search = &proc::search, \
		.getMeta = &proc::getMeta, \
		.resolveMeta = &proc::resolveMeta, \
	} \
}

const std::unordered_map<label_t, Procedure> PROCEDURES = {
	FILL(variadic),
	FILL(friendVariadic)
};


const Procedure* getProcedure(label_t name) {
	const auto it = PROCEDURES.find(name);

	if (it == PROCEDURES.end()) {
		throw StructureError{
			"UnknownProcedure",
			std::format("Cannot find procedure '{}'", name)
		};
	}

	return &it->second;
}

