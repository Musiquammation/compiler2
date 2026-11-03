#include "BuildablePrototype.hpp"

#include "Prototype.hpp"
#include "ProcResPrototype.hpp"

Prototype buildBuildableProto(
	BuildablePrototype& builder,
   	classMembers_ct members,
	classMembers_ct metaMembers,
	size_t membersLength,
	size_t metaMembersLength

) {
	return std::visit(
		[members, metaMembers, membersLength, metaMembersLength](auto& builder) -> Prototype {
			using T = std::decay_t<decltype(builder)>;

			if constexpr (std::is_same_v<T, Prototype>) {
				// The prototype is already fully built.
				return builder;
			} else if constexpr (std::is_same_v<T, ProcResPrototypeBuilder>) {
				// Build the prototype using the source class.
				auto b = builder.build(ProcResPrototypeBuilder::BuildArg{
					.members = members,
					.metaMembers = metaMembers,
					.membersLength = membersLength,
					.metaMembersLength = metaMembersLength,
				});

				return {b};
			} else {
				return {builder.build(
					members,
					metaMembers,
					membersLength,
					metaMembersLength
				)};
			}
		},
		builder
	);
}

