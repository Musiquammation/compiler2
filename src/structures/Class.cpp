
#include "Class.hpp"
#include "ClassMember.hpp"

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



void Class::describe(Logger logger) const {

	logger.print("class '%s'", this->name);

	{
		auto tab = Logger::tabulator();

		logger.print("entire: %d", this->isEntire());
		
		logger.print("size: %d (align: %d)", this->size.size, this->size.align);
		logger.print("members: %zu", this->members.getLength());

		{
			auto membersTab = Logger::tabulator();

			for (const auto& member : this->members) {
				logger.print("member '%s' @%+03d", member->name, member->placement.offset);
			}
		}

		if (this->meta) {
			logger.print("Meta:");
			auto _ = Logger::tabulator();
			this->meta->describe(logger);
		}

	}
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







struct Level {
	std::vector<ClassMember*> members = {};
	MemoryPlacer placer = {};
	bool isEntire = true;
};




static void buildMembers(
	std::vector<Level>& levels,
	ClassBuilder* builder,
	ClassBuilder* askerBuilder
) {
	size_t levelIdx = levels.size();

	logger.debug(
		"Level %ld (building)",
		levels.size(),
		builder ? builder->name : "[no builder]"
	);
	auto _ = logger.tabulator();
	{
		logger.debug("append %ld", builder ? builder->members.size() : 0);
		levels.push_back({});
	}
	
	
	Level* metaLevel;
	if (builder && builder->meta) {
		buildMembers(
			levels, 
			builder->meta,
			builder
		);
		metaLevel = &levels[levelIdx+1];
	} else {
		metaLevel = nullptr;
	}

	Level* const level = &levels[levelIdx];

	// Fill our direct members
	if (builder) {
		size_t classMembersLength = builder->members.size();
		logger.debug("Fill %ld members:", classMembersLength);
		auto _ = logger.tabulator();
		bool isEntire = level->isEntire;
		for (size_t i = 0; i < classMembersLength; ++i) {
			auto sb = builder->members[i].subBuild(
				level->members.data(),
				metaLevel ? metaLevel->members.data() : nullptr,
				i,
				metaLevel ? metaLevel->members.size() : 0
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

			level->members.push_back(
				sb.build(MemoryPlacement{
					.size = size,
					.offset = offset
				})
			);
		}

		level->isEntire = isEntire;
	}
}

static void buildMetaMembers(std::vector<Level>& levels) {
	for (size_t idx = 0; idx < levels.size(); idx++) {
		Level* level = &levels[idx];

		bool isEntire;
		if (idx + 1 < levels.size()) {
			isEntire = levels[idx+1].isEntire;
		} else {
			isEntire = true;
		}


		logger.debug("Build meta members lvl=%ld members=%ld", idx, level->members.size());
		auto _ = logger.tabulator();
		for (size_t i = 0; i < level->members.size(); i++) {
			ClassMember* m = level->members[i];
			logger.debug("Handle: %s", m->name);
			auto _ = logger.tabulator();
			auto metaOpt = m->proto.getMeta();
			if (!metaOpt.has_value())
				continue;

			auto& meta = metaOpt.value();

			MemorySize size = meta.evalQuickSize();
			if (!size.isValid() || !meta.isEntire()) {
				isEntire = false;
			}
			logger.debug("Handle: %s isEntire %d", m->name, isEntire);

			// Create the target
			label_t name = m->name;
			if (idx + 1 >= levels.size()) {
				levels.push_back({});
				level = &levels[idx]; // update reference
			}


			Level* target = &levels[idx+1];

			int offset = target->placer.add(size);
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

			target->members.push_back(new ClassMember{
				.name = name,
				.proto = meta,
				.placement = MemoryPlacement{size, offset},
			});
		}

		if (idx + 1 < levels.size()) {
			levels[idx+1].isEntire = isEntire;
		}
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
	
	logger.debug(
		"Build level %ld (size: %d %d)", 
		idx,
		levels[idx].placer.evalSize().size,
		levels[idx].placer.evalSize().align
	);

	return ClassBuilder::generateClass(
		name,
		std::move(meta),
		levels[idx].members,
		nullptr,
		levels[idx].placer.evalSize(),
		levels[idx].isEntire
	);
}

std::unique_ptr<Class> ClassBuilder::generateClass(
	label_t name,
	std::unique_ptr<Class> meta,
	std::vector<ClassMember *> members,
	const Primitive* primitive,
	MemorySize size,
	bool isEntire
) {
	return std::unique_ptr<Class>(new Class{
		name,
		std::move(meta),
		members,
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



Class::~Class() {
	// Delete members
	for (auto i: this->members) {
		delete i;
	}
}