#pragma once

#include "RealPrototype.hpp"
#include "symbol/ClassSymbol.hpp"
#include <optional>
#include <variant>
#include <vector>

struct Prototype;
struct ClassMember;
struct ProcResPrototype;

struct PEClassMember {
	const ClassMember* member;
};

struct PEMetaClassMember {
	const ClassMember* member;
};

using ProcedureEntry = std::variant<
	Prototype,
	std::shared_ptr<ProcResPrototype>,
	PEClassMember,
	PEMetaClassMember
>;

struct Procedure {
private:
	struct Entries {
		const std::vector<ProcedureEntry>& entries;

		template<typename T>
		const T& get(int idx) const; // defined in procedures.cpp
	};

public:
	using Entries = struct Entries;
	int args;
	std::optional<MemorySize> (*evalProtoSize)(
		Entries entries,
		std::vector<const RealPrototype*>& stack
	);
	ClassSymbol (*minimalClass)(Entries entries);
	label_t (*getSymbolPlaceholder)(Entries entries);
	const ClassMember* (*getReferencedMember)(Entries entries);
	std::optional<Prototype> (*search)(
		Entries entries,
		const std::map<const ClassMember *, Prototype>* settings
	);
	std::optional<Prototype> (*getMeta)(Entries entries);
	Prototype (*resolveMeta)(Entries entries);
};

struct ProcedureCall {
	const Procedure* procedure;
	std::vector<ProcedureEntry> entries;
};


