
#include "Class.hpp"
#include "ClassMember.hpp"

#include "../util/loopPtr.hpp"
#include "../util/Logger.hpp"
#include "../util/MemoryPlacement.hpp"
#include "../util/MemoryPlacer.hpp"
#include "../util/MemorySize.hpp"
#include "procedures.hpp"
#include <format>
#include <memory>
#include <sys/types.h>
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







struct Level {
	std::unique_ptr<ClassMember[]> ptr;
	size_t length;
	MemoryPlacer placer;
	int isEntire;
};




static void buildMembers(
	std::vector<Level>& levels,
	ClassBuilder* builder,
	ClassBuilder* askerBuilder
) {
	size_t extraMembersCount = 0;
	// Count members with meta
	if (askerBuilder) {
		for (auto member: askerBuilder->members) {
			if (!member.isTriviallyNoMeta()) {
				extraMembersCount++;
			}
		}
	}

	size_t levelIdx = levels.size();

	logger.debug(
		"Level %ld (building %s, extra=%ld)",
		levels.size(),
		builder ? builder->name : "[no builder]",
		extraMembersCount
	);
	auto _ = logger.tabulator();
	{
		size_t membersSize = extraMembersCount;
		if (builder) {
			membersSize += builder->members.size();
		}
		
		logger.debug("append %ld %ld", membersSize, builder ? builder->members.size() : 0);
		levels.push_back({
			.ptr = (
				membersSize > 0 ?
				std::make_unique_for_overwrite<ClassMember[]>(membersSize) :
				nullptr
			),
			.length = 0,
			.placer = {},
			.isEntire = -1
		});
	}
	
	
	Level* metaLevel;
	if (extraMembersCount > 0 || (builder && builder->meta)) {
		buildMembers(
			levels, 
			builder ? builder->meta : nullptr,
			builder
		);
		metaLevel = &levels[levelIdx+1];
	} else {
		metaLevel = nullptr;
	}

	Level* level = &levels[levelIdx];

	// Fill our direct members
	if (builder) {
		size_t classMembersLength = builder->members.size();
		logger.debug("Fill %ld members:", classMembersLength);
		auto _ = logger.tabulator();
		bool isEntire = true;
		for (size_t i = 0; i < classMembersLength; ++i) {
			auto sb = builder->members[i].subBuild(
				level->ptr.get(),
				metaLevel ? metaLevel->ptr.get() : nullptr,
				i,
				metaLevel ? metaLevel->length : 0
			);

			logger.debug("getting '%s'", sb.name);
			auto _ = logger.tabulator();

			MemorySize size = sb.proto.evalQuickSize();
			if (isEntire && (!size.isValid() || !sb.proto.isEntire())) {
				isEntire = false;
			}


			int offset = level->placer.add(size);

			if (size.isValid()){
				auto offsetStr = std::format("+{}", offset);
				logger.debug(
					"append '%s' (%d, %d) at %s",
					sb.name,
					size.size,
					size.align,
					offset < 0 ? "(?)" : offsetStr.c_str()
				);
			} else {
				auto offsetStr = std::format("+{}", offset);
				logger.debug(
					"append '%s' (invalid) at %s",
					sb.name,
					offset < 0 ? "(?)" : offsetStr.c_str()
				);
			}

			level->ptr[i] = sb.build(MemoryPlacement{
				.size = size,
				.offset = offset
			});
		}

		level->length = classMembersLength;
	}
}

static void buildMetaMembers(std::vector<Level>& levels) {
	const size_t levelsLength = levels.size() - 1;
	for (size_t idx = 0; idx < levelsLength; idx++) {
		auto& level = levels[idx];
		logger.debug("Build meta members lvl=%ld members=%ld", idx, level.length);
		auto _ = logger.tabulator();
		auto& target = levels[idx+1];
		size_t t = target.length;
		loopPtr(m, &level.ptr[0], level.length) {
			logger.debug("Handle: %s", m->name);
			auto _ = logger.tabulator();
			auto metaOpt = m->proto.resolveMeta().getMeta();
			if (!metaOpt.has_value())
				continue;

			auto& meta = metaOpt.value();

			MemorySize size = meta.evalQuickSize();
			if (!size.isValid() || !meta.isEntire()) {
				level.isEntire = false;
			}

			int offset = level.placer.add(size);
			if (size.isValid()){
				auto offsetStr = std::format("+{}", offset);
				logger.debug(
					"append(meta) '%s' (%d, %d) at %s",
					m->name,
					size.size,
					size.align,
					offset < 0 ? "(?)" : offsetStr.c_str()
				);
			} else {
				auto offsetStr = std::format("+{}", offset);
				logger.debug(
					"append(meta) '%s' (invalid) at %s",
					m->name,
					offset < 0 ? "(?)" : offsetStr.c_str()
				);
			}

			auto& r = target.ptr[t++];
			r.name = m->name;
			r.placement = MemoryPlacement{size, offset};
			r.proto = meta;
		}

		target.length = t;
	}
}


static std::unique_ptr<Class> quickGenerateClass(
	std::vector<Level>& levels,
	size_t idx,
	label_t name
) {
	std::unique_ptr<Class> meta = (
		idx < levels.size() - 1 ?
		quickGenerateClass(levels, idx+1, nullptr) :
		nullptr
	); 
	
	return ClassBuilder::generateClass(
		name,
		std::move(meta),
		levels[idx].ptr,
		levels[idx].length,
		nullptr,
		levels[idx].placer.evalSize(),
		levels[idx].isEntire != 0
	);
}

std::unique_ptr<Class> ClassBuilder::generateClass(
	label_t name,
	std::unique_ptr<Class> meta,
	std::unique_ptr<ClassMember[]>& classMembers,
	size_t classMemberLength,
	const Primitive* primitive,
	MemorySize size,
	bool isEntire
) {
	return std::make_unique<Class>(Class{
		name,
		std::move(meta),
		classMembers,
		classMemberLength,
		primitive,
		size,
		isEntire
	});
}


std::unique_ptr<Class> ClassBuilder::build() {
	std::vector<Level> levels;
	buildMembers(levels, this, nullptr);
	buildMetaMembers(levels);

	return quickGenerateClass(levels, 0, this->name);
}



