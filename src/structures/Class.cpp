#include "Class.hpp"
#include "ClassMember.hpp"
#include "VariadicPrototype.hpp"

#include "../util/Logger.hpp"
#include "../util/MemoryPlacement.hpp"
#include "../util/MemoryPlacer.hpp"
#include "../util/MemorySize.hpp"
#include <format>

static auto logger = getLogger("class");

Class::Class(
	label_t name,
	const Primitive* primitive,
	MemorySize size
): 
	name{name},
	primitive{primitive},
	size{size},
	_isEntire{true}
{};




Class Class::create(
	label_t name,
	std::unique_ptr<Class> meta,
	std::vector<ClassMemberBuilder>& members
) {
	logger.debug("Create '%s'", name);
	auto _ = logger.tabulator();
	MemoryPlacer placer;

	size_t classMembersLength = members.size();
	std::unique_ptr<ClassMember[]> membersArr =
		std::make_unique<ClassMember[]>(classMembersLength);

	bool isEntire = true;
	for (size_t i = 0; i < members.size(); ++i) {
		auto& builder = members[i];
		logger.debug("getting '%s'", builder.name);
		auto _ = logger.tabulator();

		MemorySize size = builder.proto.evalSize();
		if (isEntire && (!size.isValid() || !builder.proto.isEntire())) {
			isEntire = false;
		}


		int offset = placer.add(size);

		if (size.isValid()){
			auto offsetStr = std::format("+{}", offset);
			logger.debug(
				"append '%s' (%d, %d) at %s",
				builder.name,
				size.size,
				size.align,
				offset < 0 ? "(?)" : offsetStr.c_str()
			);
		} else {
			auto offsetStr = std::format("+{}", offset);
			logger.debug(
				"append '%s' (invalid) at %s",
				builder.name,
				offset < 0 ? "(?)" : offsetStr.c_str()
			);
		}

		membersArr[i] = builder.build(MemoryPlacement{
			.size = size,
			.offset = offset
		});
	}

	{
		auto fsize = placer.evalSize();
		if (fsize.isValid()) {
			logger.debug("Final size: %d %d", fsize.size, fsize.align);

		} else {
			logger.debug("Final size: (invalid)");
		}
	}

	return Class{
		name,
		std::move(meta),
		membersArr,
		classMembersLength,
		nullptr,
		placer.evalSize(),
		isEntire
	};
}

Class Class::createPrimitive(const Primitive* p, label_t name) {
	return Class{
		name,
		p,
		{
			.size = p->size,
			.align = p->size
		}
	};
}
