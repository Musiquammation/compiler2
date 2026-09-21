#include "Class.hpp"
#include "ClassMember.hpp"
#include "VariadicPrototype.hpp"

#include "../util/Logger.hpp"
#include "../util/MemoryPlacement.hpp"
#include "../util/MemoryPlacer.hpp"
#include "../util/MemorySize.hpp"
#include <format>
#include <memory>
#include <optional>
#include <vector>

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




std::unique_ptr<Class> ClassBuilder::build() const {
	logger.debug("Create '%s'", name);
	auto _ = logger.tabulator();
	MemoryPlacer placer;

	size_t classMembersLength = this->members.size();
	std::unique_ptr<ClassMember[]> membersArr =
		std::make_unique<ClassMember[]>(classMembersLength);



	// Fill metaBuilder
	std::unique_ptr<Class> metaClass;
	{
		std::vector<ClassMemberBuilder> members;
		members.reserve(this->members.size());
		for (auto& i: this->members) {
			auto builder = i.getMeta();
			if (!builder.has_value()) {continue;}

			members.push_back(std::move(*builder));
		}


		const ClassBuilder* meta = this->meta;
		if (meta || !members.empty()) {
			ClassBuilder builder = {
				.name = nullptr,
				.members = std::move(members),
				.meta = meta ? meta->meta : nullptr
			};

			// Add members of the meta
			if (meta) {
				builder.members.insert(
					builder.members.begin(),
					meta->members.begin(),
					meta->members.end()
				);
			}
			metaClass = builder.build();
		} else {
			metaClass = nullptr;
		}
	}



	bool isEntire = true;
	// Fill our members
	for (size_t i = 0; i < this->members.size(); ++i) {
		auto builder = this->members[i].subBuild(metaClass.get());
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

	return std::make_unique<Class>(Class{
		this->name,
		std::move(metaClass),
		membersArr,
		classMembersLength,
		nullptr,
		placer.evalSize(),
		isEntire
	});
}