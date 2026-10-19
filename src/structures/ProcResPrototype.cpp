#include "ProcResPrototype.hpp"

#include "Class.hpp"
#include "StructureError.hpp"
#include "procedures.hpp"

#include "../poolBase.hpp"
#include "../util/Logger.hpp"

#include <cstdio>
#include <memory>
#include <vector>
#include <format>


static auto logger = getLogger("procres");


std::optional<MemorySize> ProcResPrototype::evalProtoSize(
	std::vector<const RealPrototype*>& stack
) const {
	logger.debug("Running evalProtoSize");
	auto _ = logger.tabulator();

	return this->proc.procedure->evalProtoSize({this->proc.entries}, stack);
}

ClassSymbol ProcResPrototype::getMinimalClass() const {
	logger.debug("Running getMinimalClass");
	auto _ = logger.tabulator();

	return this->proc.procedure->minimalClass({this->proc.entries});
}

label_t ProcResPrototype::getSymbolPlaceholder() const {
	logger.debug("Running getSymbolPlaceholder");
	auto _ = logger.tabulator();

	return this->proc.procedure->getSymbolPlaceholder({this->proc.entries});
}

const ClassMember* ProcResPrototype::getReferencedMember() const {
	logger.debug("Running getReferencedMember");
	auto _ = logger.tabulator();

	return this->proc.procedure->getReferencedMember({this->proc.entries});
}

std::optional<Prototype> ProcResPrototype::search(
	const std::map<const ClassMember *, Prototype>* settings
) const {
	logger.debug("Running search");
	auto _ = logger.tabulator();

	return this->proc.procedure->search({this->proc.entries}, settings);
}

std::optional<Prototype> ProcResPrototype::getMeta() const {
	logger.debug("Running getMeta");
	auto _ = logger.tabulator();

	return this->proc.procedure->getMeta({this->proc.entries});
}

Prototype ProcResPrototype::resolveMeta() const {
	logger.debug("Running resolveMeta");
	auto _ = logger.tabulator();

	return this->proc.procedure->resolveMeta({this->proc.entries});
}

bool ProcResPrototype::isMetaResolutionDistinct() const {
	logger.debug("Running isMetaResolutionDistinct");
	auto _ = logger.tabulator();

	return true;
}

bool ProcResPrototype::isTriviallyNoMeta() const {
	logger.debug("Running isTriviallyNoMeta");
	auto _ = logger.tabulator();

	/// TODO: optimize
	return !this->getMeta().has_value();
}





PEClassMember PEClassMemberBuilder::build(
	classMembers_ct metaMembers,
	size_t metaMembersLength
) const {
	for (size_t i = 0; i < metaMembersLength; i++) {
		if (metaMembers[i]->name == this->name) {
			return {metaMembers[i]};
		}
	}

	return {nullptr};
}

PEMetaClassMember PEMetaClassMemberBuilder::build(
	classMembers_ct constructingMembers,
	size_t constructingMembersLength
) const {
	for (size_t i = 0; i < constructingMembersLength; ++i) {
		if (constructingMembers[i]->name == this->name) {
			return {constructingMembers[i]};
		}
	}

	throw StructureError{
		"ConstructingClassMemberNotResolved",
		std::format("Could not resolve member '{}'", this->name)
	};
}

std::shared_ptr<ProcResPrototype> ProcResPrototypeBuilder::build(
	ProcResPrototypeBuilder::BuildArg arg
) {
	std::vector<ProcedureEntry> entries;
	entries.reserve(args.size());

	for (auto& argument : this->args) {
		std::visit([&entries, &arg](auto& value) {
			using T = std::decay_t<decltype(value)>;

			if constexpr (std::is_same_v<T, Prototype>) {
				entries.push_back(value);
			} else if constexpr (std::is_same_v<T, std::shared_ptr<ProcResPrototype>>) {
				entries.push_back(std::move(value));
			} else if constexpr (std::is_same_v<T, std::shared_ptr<ProcResPrototypeBuilder>>) {
				entries.push_back(std::move(value->build(arg)));
			} else if constexpr (std::is_same_v<T, PEClassMember>) {
				entries.push_back(value);
			} else if constexpr (std::is_same_v<T, PEMetaClassMember>) {
				entries.push_back(value);
			} else if constexpr (std::is_same_v<T, PEClassMemberBuilder>) {
				if (arg.membersLength == FAKE_CONSTRUCTING_MEMBERS_LENGTH) {
					throw StructureError{
						"InvalidProcResBuildArg",
						"Constructing members were excepted, but got a source class"
					};
				}

				entries.push_back({
					value.build(
						arg.members,
						arg.membersLength
					)
				});
			} else if constexpr (std::is_same_v<T, PEMetaClassMemberBuilder>) {
				entries.push_back(value.build(arg.metaMembers, arg.metaMembersLength));
			}
		}, argument);
	}

	return std::make_unique<ProcResPrototype>(
		ProcResPrototype{
			{
				procedure,
				std::move(entries)
			}
		}
	);
}




 
using ProcArg = ProcResPrototypeBuilder::ProcArg;

ProcResPrototypeBuilder ProcResPrototypeBuilder::variadic(label_t member) {
	ProcResPrototypeBuilder result{
		.procedure = getProcedure(poolBase.variadic)
	};

	result.args.emplace_back(
		PEMetaClassMemberBuilder{member}
	);

	return result;
}

