#include "Class.hpp"
#include "ClassMember.hpp"
#include "RealPrototype.hpp"
#include "VariadicPrototype.hpp"

#include "../util/Logger.hpp"
#include "../util/MemoryPlacement.hpp"
#include "../util/MemoryPlacer.hpp"
#include "../util/MemorySize.hpp"

static auto logger = getLogger("class");

Class Class::create(
	label_t name,
	std::unique_ptr<Class> meta,
	std::vector<ClassMemberBuilder>& members
) {
	MemoryPlacer placer;

	size_t classMembersLength = members.size();
	std::unique_ptr<ClassMember[]> membersArr =
		std::make_unique<ClassMember[]>(classMembersLength);

	for (size_t i = 0; i < members.size(); ++i) {
		auto& builder = members[i];
		MemorySize size = builder.proto.getQuickSize();

		membersArr[i] = builder.build(MemoryPlacement{
			.size = size,
			.offset = placer.add(size)
		});
	}

	return Class{
		name,
		std::move(meta),
		std::move(membersArr),
		nullptr,
		classMembersLength,
		placer.evalSize()
	};
}

Class Class::createPrimitive(const Primitive* p, label_t name) {
	return Class{
		name,
		nullptr,
		nullptr,
		p,
		0,
		{
			.size = p->size,
			.align = p->size
		}
	};
}
