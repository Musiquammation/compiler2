#include "ClassMember.hpp"
#include "Prototype.hpp"
#include "StructureError.hpp"
#include "RealPrototype.hpp"
#include <format>
#include <optional>
#include <variant>

ClassMember ClassMemberSubBuilder::build(MemoryPlacement placement) {
	return ClassMember{
		name,
		std::move(proto),
		std::move(placement)
	};
}


std::optional<ClassMemberBuilder> ClassMemberBuilder::getMeta() const {
	using Ret = std::optional<std::variant<Prototype, VariadicPrototypeBuilder>>;

	std::shared_ptr<VariadicPrototype>* vp = nullptr;
	Ret proto;

	if (std::holds_alternative<Prototype>(this->proto)) {
		auto& p = std::get<Prototype>(this->proto);

		if (std::holds_alternative<std::shared_ptr<RealPrototype>>(p)) {
			auto& rp = std::get<std::shared_ptr<RealPrototype>>(p);

			auto m = rp->buildMeta();
			if (m == nullptr)
				return std::nullopt;

			proto = Prototype{m};
			goto handle_vp;
		}

		// vp = &std::get<std::shared_ptr<VariadicPrototype>>(p);
		throw StructureError{
			"TODO",
			std::format("Asked meta of variadic")
		};
		goto handle_vp;
	}

	// VariadicPrototypeBuilder
	handle_vp:
	if (vp != nullptr) {
		throw StructureError{
			"TODO",
			std::format("Asked meta of variadic")
		};
	}

	if (proto.has_value()) {
		return ClassMemberBuilder{
			.name = this->name,
			.proto = std::move(proto.value()),
		};
	}

	return std::nullopt;
}

ClassMemberSubBuilder ClassMemberBuilder::subBuild(const Class* source) const {
	return ClassMemberSubBuilder{
		name,
		std::visit(
			[source](const auto& builder) -> Prototype {
				using T = std::decay_t<decltype(builder)>;

				if constexpr (std::is_same_v<T, Prototype>) {
					// The prototype is already fully built.
					return builder;
				} else {
					// Build the prototype using the source class.
					return {builder.build(source)};
				}
			},
			proto
		)
	};
}

