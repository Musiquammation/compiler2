#include "ClassMember.hpp"

#include "ProcResPrototype.hpp"
#include "Prototype.hpp"
#include "RealPrototype.hpp"

#include <variant>

ClassMember* ClassMemberSubBuilder::build(MemoryPlacement placement) {
	return new ClassMember{
		name,
		std::move(proto),
		std::move(placement)
	};
}

ClassMemberSubBuilder ClassMemberBuilder::subBuild(
	classMembers_ct members,
	classMembers_ct metaMembers,
	size_t membersLength,
	size_t metaMembersLength
) {
	return ClassMemberSubBuilder{
		this->name,
		std::visit(
			[members, metaMembers, membersLength, metaMembersLength](auto& builder) -> Prototype {
				using T = std::decay_t<decltype(builder)>;

				if constexpr (std::is_same_v<T, Prototype>) {
					// The prototype is already fully built.
					return builder;
				} else {
					// Build the prototype using the source class.
					auto b = builder.build(ProcResPrototypeBuilder::BuildArg{
						.members = members,
						.metaMembers = metaMembers,
						.membersLength = membersLength,
						.metaMembersLength = metaMembersLength,
					});

					return {b};
				}
			},
			this->proto
		)
	};
}
