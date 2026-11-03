#include "ClassMember.hpp"

#include "BuildablePrototype.hpp"
#include "Prototype.hpp"
#include "RealPrototype.hpp"


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
		buildBuildableProto(
			this->proto,
			members,
			metaMembers,
			membersLength,
			metaMembersLength
		)
	};
}
