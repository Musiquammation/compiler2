#pragma once

#include "Procedure.hpp"
#include "Prototype.hpp"
#include "RealPrototype.hpp"
#include <cstdint>
#include <memory>
#include <variant>
#include <vector>

struct ProcResPrototype {
	ProcedureCall proc;
	std::optional<MemorySize> evalProtoSize(
		std::vector<const RealPrototype*>& stack
	) const;
	ClassSymbol getMinimalClass() const;
	label_t getSymbolPlaceholder() const;
	const ClassMember* getReferencedMember() const;
	std::optional<Prototype> search(
		const std::map<const ClassMember *, Prototype>* settings
	) const;
	std::optional<Prototype> getMeta() const;
	Prototype resolveMeta() const;

	bool isMetaResolutionDistinct() const;
	bool isTriviallyNoMeta() const;
};

struct PEClassMemberBuilder {
	label_t name;
	PEClassMember build(
		const ClassMember* constructingMembers,
		size_t constructingMembersLength
	) const;
};

struct PEMetaClassMemberBuilder {
	label_t name;
	PEMetaClassMember build(
		const ClassMember* metaMembers,
		size_t metaMembersLength
	) const;
};

struct ProcResPrototypeBuilder {
	using ProcArg = std::variant<
		Prototype,
		std::shared_ptr<ProcResPrototype>,
		std::shared_ptr<ProcResPrototypeBuilder>,
		PEClassMember,
		PEMetaClassMember,
		PEClassMemberBuilder,
		PEMetaClassMemberBuilder
	>;
	const Procedure* procedure;
	std::vector<ProcArg> args;

	static constexpr size_t FAKE_CONSTRUCTING_MEMBERS_LENGTH = SIZE_MAX;

	struct BuildArg {
		const ClassMember* members;
		const ClassMember* metaMembers;
		size_t membersLength;
		size_t metaMembersLength;

	};

	std::shared_ptr<ProcResPrototype> build(BuildArg arg);

	static ProcResPrototypeBuilder variadic(label_t member);
};
