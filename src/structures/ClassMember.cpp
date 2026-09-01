#include "ClassMember.hpp"

ClassMember ClassMemberBuilder::build(MemoryPlacement placement) {
	return ClassMember{
		.name = name,
		.proto = proto,
		.placement = placement
	};
}
