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
	auto proto = std::visit([](const auto& proto) -> Ret {
		using T = std::decay_t<decltype(proto)>;

		if constexpr (std::is_same_v<T, Prototype>) {
			return std::visit([](const auto& proto) -> Ret {
				using T = std::decay_t<decltype(proto)>;

				if constexpr (std::is_same_v<T, std::shared_ptr<RealPrototype>>) {
					auto m = proto->buildMeta();
					if (m == nullptr)
						return std::nullopt;

					return Prototype{m};

				} else {
					throw StructureError{
						"TODO",
						std::format("Asked meta of variadic")
					};

				}
			}, proto);

		} else {
			throw StructureError{
				"TODO",
				std::format("Asked meta of variadic (builder)")
			};
		}
		
	}, this->proto);

	if (proto.has_value()) {
		return ClassMemberBuilder{
			.name = this->name,
			.proto = *proto,
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

