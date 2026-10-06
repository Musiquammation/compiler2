#include "ClassMember.hpp"

#include "ProcResPrototype.hpp"
#include "Prototype.hpp"
#include "RealPrototype.hpp"

#include <variant>
#include "../util/Logger.hpp"

ClassMember ClassMemberSubBuilder::build(MemoryPlacement placement) {
	return ClassMember{
		name,
		std::move(proto),
		std::move(placement)
	};
}

ClassMemberSubBuilder ClassMemberBuilder::subBuild(
	const ClassMember* members,
	const ClassMember* metaMembers,
	size_t membersLength,
	size_t metaMembersLength
) {
	getLogger("classMember").info("azry %p", this->name);
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

bool ClassMemberBuilder::isTriviallyNoMeta() const {
	return std::visit([](auto& proto) -> bool {
		using T = std::decay_t<decltype(proto)>;

		if constexpr (std::is_same_v<T, Prototype>) {
			return proto.isTriviallyNoMeta();
		} else {
			return false;
		}
	}, proto);
}

