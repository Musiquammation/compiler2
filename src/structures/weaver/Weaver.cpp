#include "Weaver.hpp"

#include "../../util/Logger.hpp"

#include "../Variable.hpp"
#include "../Function.hpp"
#include "../StructureError.hpp"

#include <cstddef>
#include <cstdint>
#include <cstring>
#include <fstream>
#include <sys/types.h>
#include <unordered_map>
#include <vector>
#include <algorithm>
#include <cstddef>
#include <stdexcept>
#include <utility>

static auto logger = getLogger("weaver");


#define STACK_CAPACITY ((size_t)1024)

struct Weaver::WVariable {
	Variable v;
	var_t meta;
	void* data = nullptr;
};

struct Weaver::Level {
	struct Stack {
		void* data;
		size_t offset;
		size_t capacity;

		explicit Stack(size_t capacity);
		~Stack();

		Stack(const Stack&) = delete;
		Stack& operator=(const Stack&) = delete;

		Stack(Stack&& other) noexcept;
		Stack& operator=(Stack&& other) noexcept;
	};

	struct Scope {
		size_t variable;
		size_t stackOffset;
		size_t stackIdx;
	};

	std::vector<Instruction> instructions;
	std::vector<WVariable> variables;
	std::vector<Scope> scopes;
	std::vector<var_t> args;
	std::unordered_map<label_t, var_t> names;
	std::vector<Stack> stacks;

	size_t stackIdx = SIZE_MAX;

	void incScope();
	void decScope();
	WVariable& get(var_t v);

	void* allocate(size_t size, size_t offset);
};

// Stack

Weaver::Level::Stack::Stack(size_t capacity)
	: data{::operator new(capacity)},
	  offset{0},
	  capacity{capacity} {
}

Weaver::Level::Stack::~Stack() {
	::operator delete(data);
}

Weaver::Level::Stack::Stack(Stack&& other) noexcept
	: data{std::exchange(other.data, nullptr)},
	  offset{other.offset},
	  capacity{other.capacity} {
}

Weaver::Level::Stack& Weaver::Level::Stack::operator=(Stack&& other) noexcept {
	if (this != &other) {
		::operator delete(data);

		data = std::exchange(other.data, nullptr);
		offset = other.offset;
		capacity = other.capacity;
	}

	return *this;
}


// Allocation

void* Weaver::Level::allocate(size_t size, size_t offset) {
	if (size > SIZE_MAX - offset) {
		throw createStructureError(
			"WeaverAllocationOverflow",
			"Stack allocation overflow"
		);
	}

	size_t required = size + offset;

	if (stackIdx == SIZE_MAX) {
		stacks.emplace_back(std::max(STACK_CAPACITY, required));
		stackIdx = 0;
	}

	auto& stack = stacks[stackIdx];

	if (stack.offset > stack.capacity ||
		required > stack.capacity - stack.offset) {

		++stackIdx;

		if (stackIdx == stacks.size()) {
			stacks.emplace_back(std::max(STACK_CAPACITY, required));
		}

		stacks[stackIdx].offset = 0;
	}

	auto& current = stacks[stackIdx];

	auto* address = static_cast<std::byte*>(current.data)
		+ current.offset + offset;

	current.offset += required;

	return address;
}


// Scopes

void Weaver::Level::incScope() {
	scopes.push_back({
		.variable = variables.size(),
		.stackOffset = stackIdx == SIZE_MAX ? 0 : stacks[stackIdx].offset,
		.stackIdx = stackIdx
	});
}

void Weaver::Level::decScope() {
	if (scopes.empty()) {
		throw createStructureError(
			"WeaverScopeUnderflow",
			"No scope to close"
		);
	}

	auto scope = scopes.back();

	for (size_t i = scope.variable; i < variables.size(); ++i) {
		if (variables[i].v.name) {
			names.erase(variables[i].v.name);
		}
	}

	variables.erase(
		variables.begin() + scope.variable,
		variables.end()
	);

	stackIdx = scope.stackIdx;

	if (stackIdx != SIZE_MAX) {
		stacks[stackIdx].offset = scope.stackOffset;
	}

	scopes.pop_back();
}


// Variables

Weaver::WVariable& Weaver::Level::get(var_t v) {
	if (v >= variables.size()) {
		throw createStructureError(
			"WeaverVariableOutOfBounds",
			std::format("Variable index out of bounds: {}", v)
		);
	}

	return variables[v];
}






Weaver::Weaver(
	std::vector<Argument>& args, std::ofstream& stream
): args{args}, stream{stream} {
	
}

Weaver::~Weaver() {

}


Weaver::Level& Weaver::getLevel(size_t level) {
	if (this->levels.size() <= level)
		this->levels.resize(level + 1);

	return this->levels[level];
}




Weaver::var_t Weaver::create(label_t name, Prototype& proto, size_t level, bool createMeta) {
	logger.debug("create %s %d", name, (int)level);
	var_t metaVar;
	if (createMeta) {
		auto meta = proto.getMeta();
		if (meta){
			metaVar = this->create(nullptr, *meta, level+1, true);
		}
	} else {
		metaVar = NO_VAR;
	}

	auto& l = this->getLevel(level);
	var_t id = l.variables.size();


	logger.debug("gotId %ld for %ld meta=%ld", id, level, metaVar);
	if (name) {
		l.names[name] = id;
	}

	void* data;
	if (level == 0) {
		data = nullptr;
	} else {
		auto size = proto.evalQuickSize();
		data = l.allocate(size.size, size.align);
	}

	l.variables.emplace_back(Variable{name, proto}, metaVar, data);
	return id;
}



Weaver::var_t Weaver::search(label_t name, size_t lvl) {
	auto& level = this->getLevel(lvl);
	auto it = level.names.find(name);
	if (it == level.names.end()) {
		/// TODO: every level will have arguments 
		if (lvl == 0) {
			for (auto& arg: this->args) {
				if (arg.name == name) {
					return arg.varId;
				}
			}
		}
		throw createStructureError(
			"WeaverVariableNotFound",
			std::format("Variable not found: {}", name)
		);
	}

	return it->second;
}

Weaver::var_t Weaver::takeArgument(var_t v, size_t dstLevel, size_t srcLevel) {
	auto& dst = this->getLevel(dstLevel);
	auto& src = this->getLevel(srcLevel);

	auto& source = src.get(v);
	auto arg = this->create(
		nullptr,
		source.v.proto,
		dstLevel,
		false
	);

	if (source.meta) {
		auto meta = this->takeArgument(source.meta, dstLevel+1, srcLevel+1);
		auto& argument =this->getLevel(dstLevel).get(arg);
		argument.meta = meta;
	}

	auto argIdx = dst.args.size();
	dst.args.push_back(arg);

	src.instructions.push_back(ISlip{
		.src = v,
		.dst = argIdx,
		.level = dstLevel
	});

	return arg;
}

Weaver::var_t Weaver::meta(var_t var, size_t level) {
	return this->levels[level].get(var).meta;
}

Variable& Weaver::variable(var_t var, size_t level) {
	return this->levels[level].get(var).v;
}








void Weaver::append(const Weaver::IncScope& ins, size_t lvl) {
	auto& level = this->levels[lvl];
	level.instructions.push_back(ins);
	
	level.incScope();
}

void Weaver::append(const Weaver::DecScope& ins, size_t lvl) {
	auto& level = this->levels[lvl];
	level.instructions.push_back(ins);

	level.decScope();
}

void Weaver::append(const Weaver::ISet& ins, size_t lvl) {
	auto& level = this->levels[lvl];
	level.instructions.push_back(ins);

	void* ptr = (
		(uint8_t*)level.get(ins.dst.variable).data
		+ ins.dst.offset
	);

	std::memcpy(ptr, &ins.arg, ins.size);


	// Log data
	switch (ins.size) {
	case 1:
		logger.debug("set %p to %hhu (%zu bytes)", ptr, *(uint8_t*)ptr, ins.size);
		break;
	case 2:
		logger.debug("set %p to %hu (%zu bytes)", ptr, *(uint16_t*)ptr, ins.size);
		break;
	case 4:
		logger.debug("set %p to %u (%zu bytes)", ptr, *(uint32_t*)ptr, ins.size);
		break;
	case 8:
		logger.debug("set %p to %llu (%zu bytes)", ptr, *(uint64_t*)ptr, ins.size);
		break;
	default:
		logger.debug("set to %p (%zu bytes)", ptr, ins.size);
		break;
}
}



















