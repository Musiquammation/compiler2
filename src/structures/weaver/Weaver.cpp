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
#include <deque>
#include <unordered_map>
#include <vector>
#include <algorithm>
#include <cstddef>
#include <type_traits>
#include <utility>
#include <memory>
#include <new>
#include <format>

static auto logger = getLogger("weaver");
static auto exec_logger = getLogger("interpreter");


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

	struct AlignedFree {
		size_t align;
		void operator()(void* p) const {
			::operator delete(p, std::align_val_t{align});
		}
	};

	std::vector<Instruction> instructions;
	// deque: keeps references valid and never copies Variable (raw owned pointer)
	std::deque<WVariable> variables;
	// Slipped variables live in the level's main scope: they are never
	// erased by decScope and never shift the ids of regular variables.
	// Their ids are tagged with SLIP_BIT.
	std::deque<WVariable> slips;
	std::vector<std::unique_ptr<void, AlignedFree>> slipBuffers;
	std::vector<Scope> scopes;
	std::vector<var_t> args;
	std::unordered_map<label_t, var_t> names;
	std::vector<Stack> stacks;

	size_t stackIdx = SIZE_MAX;

	Level() = default;
	Level(Level&&) noexcept = default;
	Level& operator=(Level&&) noexcept = default;

	void incScope();
	void decScope();
	WVariable& get(var_t v);
	var_t search(label_t name) const;

	static bool isSlip(var_t v) { return v != NO_VAR && (v & SLIP_BIT); }
	var_t createSlip(label_t name, Prototype& proto, bool allocateData);
	size_t pushArg(var_t v);

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
	if (isSlip(v)) {
		size_t idx = v & ~SLIP_BIT;
		if (idx >= slips.size()) {
			throw createStructureError(
				"WeaverVariableOutOfBounds",
				std::format("Slip index out of bounds: {}", idx)
			);
		}
		return slips[idx];
	}

	if (v >= variables.size()) {
		throw createStructureError(
			"WeaverVariableOutOfBounds",
			std::format("Variable index out of bounds: {}", v)
		);
	}

	return variables[v];
}

Weaver::var_t Weaver::Level::search(label_t name) const {
	auto it = names.find(name);
	if (it != names.end()) {
		return it->second;
	}

	return NO_VAR;
}

Weaver::var_t Weaver::Level::createSlip(label_t name, Prototype& proto, bool allocateData) {
	var_t id = slips.size() | SLIP_BIT;

	void* data = nullptr;
	if (allocateData) {
		auto size = proto.evalQuickSize();
		size_t bytes = std::max<size_t>(1, (size_t)size.size);
		size_t align = std::max<size_t>(alignof(std::max_align_t), (size_t)size.align);

		// Own heap buffer: outlives every scope of this level
		slipBuffers.emplace_back(
			::operator new(bytes, std::align_val_t{align}),
			AlignedFree{align}
		);
		data = slipBuffers.back().get();
		std::memset(data, 0, bytes);
	}

	if (name) {
		names[name] = id;
	}

	slips.emplace_back(Variable{name, proto}, NO_VAR, data);
	return id;
}

size_t Weaver::Level::pushArg(var_t v) {
	args.push_back(v);
	return args.size() - 1;
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
	var_t metaVar = NO_VAR;
	if (createMeta) {
		auto meta = proto.getMeta();
		if (meta){
			metaVar = this->create(nullptr, *meta, level+1, true);
		}
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
	auto found = level.search(name);
	if (found == NO_VAR) {
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

	return found;
}

Weaver::var_t Weaver::slip(var_t v, size_t dstLevel, size_t srcLevel) {
	// Ensure both levels exist before taking references (resize invalidates them)
	this->getLevel(std::max(dstLevel, srcLevel));

	// Metas first: the recursive call may resize `levels`
	var_t srcMeta = this->levels[srcLevel].get(v).meta;
	var_t dstMeta = NO_VAR;
	if (srcMeta != NO_VAR) {
		dstMeta = this->slip(srcMeta, dstLevel+1, srcLevel+1);
	}

	auto& srcLvl = this->levels[srcLevel];
	auto& dstLvl = this->levels[dstLevel];
	auto& source = srcLvl.get(v);

	// Level 0 is the runtime level: no data is held there
	bool hasData = dstLevel != 0;
	auto arg = dstLvl.createSlip(nullptr, source.v.proto, hasData);
	auto& target = dstLvl.get(arg);
	target.meta = dstMeta;

	// Actually pass the value to the destination level
	if (hasData && source.data && target.data) {
		size_t size = (size_t)source.v.proto.evalQuickSize().size;
		std::memcpy(target.data, source.data, size);
	}

	auto argIdx = dstLvl.pushArg(arg);

	srcLvl.instructions.push_back(ISlip{
		.src = v,
		.dst = argIdx,
		.level = dstLevel
	});

	exec_logger.debug(
		"slip %zu@%zu to arg %zu (slip %zu)@%zu",
		v, srcLevel, argIdx, arg & ~SLIP_BIT, dstLevel
	);

	return arg;
}

Weaver::var_t Weaver::meta(var_t var, size_t level) {
	return this->levels[level].get(var).meta;
}

Variable& Weaver::variable(var_t var, size_t level) {
	return this->levels[level].get(var).v;
}








void Weaver::incScope(size_t lvl) {
	auto& level = this->levels[lvl];
	level.instructions.push_back(Weaver::IncScope{});
	
	level.incScope();
}

void Weaver::decScope(size_t lvl) {
	auto& level = this->levels[lvl];
	level.instructions.push_back(Weaver::DecScope{});

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
		exec_logger.debug("set %p to %hhu (%zu bytes)", ptr, *(uint8_t*)ptr, ins.size);
		break;
	case 2:
		exec_logger.debug("set %p to %hu (%zu bytes)", ptr, *(uint16_t*)ptr, ins.size);
		break;
	case 4:
		exec_logger.debug("set %p to %u (%zu bytes)", ptr, *(uint32_t*)ptr, ins.size);
		break;
	case 8:
		exec_logger.debug("set %p to %llu (%zu bytes)", ptr, *(uint64_t*)ptr, ins.size);
		break;
	default:
		exec_logger.debug("set to %p (%zu bytes)", ptr, ins.size);
		break;
}
}



void Weaver::append(const Weaver::ICpy& ins, size_t lvl) {
	auto& level = this->levels[lvl];
	level.instructions.push_back(ins);

	void* dst = (uint8_t*)level.get(ins.dst.variable).data + ins.dst.offset;
	const void* src = (uint8_t*)level.get(ins.src.variable).data + ins.src.offset;
	std::memmove(dst, src, ins.size);

	exec_logger.debug("cpy %p to %p (%zu bytes)", src, dst, ins.size);
}

// Integer operations
//
// Values are handled as signed integers of the variable's size.
// Wrapping arithmetic goes through the unsigned type: same bits, no UB.

// Calls op.template operator()<T>() with the signed integer type of `size`
template<typename Op>
static void dispatchSize(size_t size, const char* name, Op&& op) {
	switch (size) {
	case 1: op.template operator()<int8_t>(); break;
	case 2: op.template operator()<int16_t>(); break;
	case 4: op.template operator()<int32_t>(); break;
	case 8: op.template operator()<int64_t>(); break;
	default:
		throw createStructureError(
			"WeaverInvalidSize",
			std::format("{} on unsupported size {}", name, size)
		);
	}
}

static uint64_t readUnsigned(const void* src, size_t size) {
	switch (size) {
	case 1: { uint8_t v;  std::memcpy(&v, src, 1); return v; }
	case 2: { uint16_t v; std::memcpy(&v, src, 2); return v; }
	case 4: { uint32_t v; std::memcpy(&v, src, 4); return v; }
	case 8: { uint64_t v; std::memcpy(&v, src, 8); return v; }
	default:
		throw createStructureError(
			"WeaverInvalidSize",
			std::format("Cannot read integer of size {}", size)
		);
	}
}

// dst = op(dst, src), both operands of the same size
template<typename W, typename Op>
static void binaryOp(W& dst, W& src, const char* name, Op op) {
	size_t size = (size_t)dst.v.proto.evalQuickSize().size;

	if ((size_t)src.v.proto.evalQuickSize().size != size) {
		throw createStructureError(
			"WeaverSizeMismatch",
			std::format("{} operands differ in size ({})", name, size)
		);
	}

	dispatchSize(size, name, [&]<typename T>() {
		T a, b;
		std::memcpy(&a, dst.data, sizeof(T));
		std::memcpy(&b, src.data, sizeof(T));
		T r = op.template operator()<T>(a, b);
		std::memcpy(dst.data, &r, sizeof(T));
		exec_logger.debug("%s %p = %lld (%zu bytes)", name, dst.data, (long long)r, sizeof(T));
	});
}

// dst = op(dst, count), the shift count may have any integer size
template<typename W, typename Op>
static void shiftOp(W& dst, W& src, const char* name, Op op) {
	size_t size = (size_t)dst.v.proto.evalQuickSize().size;
	uint64_t count = readUnsigned(src.data, (size_t)src.v.proto.evalQuickSize().size);

	dispatchSize(size, name, [&]<typename T>() {
		T a;
		std::memcpy(&a, dst.data, sizeof(T));
		T r = op.template operator()<T>(a, count);
		std::memcpy(dst.data, &r, sizeof(T));
		exec_logger.debug("%s %p = %lld (%zu bytes)", name, dst.data, (long long)r, sizeof(T));
	});
}

// dst = op(dst)
template<typename W, typename Op>
static void unaryOp(W& dst, const char* name, Op op) {
	size_t size = (size_t)dst.v.proto.evalQuickSize().size;

	dispatchSize(size, name, [&]<typename T>() {
		T a;
		std::memcpy(&a, dst.data, sizeof(T));
		T r = op.template operator()<T>(a);
		std::memcpy(dst.data, &r, sizeof(T));
		exec_logger.debug("%s %p = %lld (%zu bytes)", name, dst.data, (long long)r, sizeof(T));
	});
}

template<typename T>
using U_t = std::make_unsigned_t<T>;

static void checkDivisor(int64_t b, const char* name) {
	if (b == 0) {
		throw createStructureError(
			"WeaverDivisionByZero",
			std::format("{} by zero", name)
		);
	}
}


void Weaver::append(const Weaver::IAdd& ins, size_t lvl) {
	auto& level = this->levels[lvl];
	level.instructions.push_back(ins);

	binaryOp(level.get(ins.dst), level.get(ins.src), "add", []<typename T>(T a, T b) {
		return (T)((U_t<T>)a + (U_t<T>)b);
	});
}

void Weaver::append(const Weaver::ISub& ins, size_t lvl) {
	auto& level = this->levels[lvl];
	level.instructions.push_back(ins);

	binaryOp(level.get(ins.dst), level.get(ins.src), "sub", []<typename T>(T a, T b) {
		return (T)((U_t<T>)a - (U_t<T>)b);
	});
}

void Weaver::append(const Weaver::IMul& ins, size_t lvl) {
	auto& level = this->levels[lvl];
	level.instructions.push_back(ins);

	binaryOp(level.get(ins.dst), level.get(ins.src), "mul", []<typename T>(T a, T b) {
		return (T)((U_t<T>)a * (U_t<T>)b);
	});
}

void Weaver::append(const Weaver::IDiv& ins, size_t lvl) {
	auto& level = this->levels[lvl];
	level.instructions.push_back(ins);

	binaryOp(level.get(ins.dst), level.get(ins.src), "div", []<typename T>(T a, T b) {
		checkDivisor(b, "div");
		// MIN / -1 overflows: wrap to MIN
		if (b == -1) return (T)((U_t<T>)0 - (U_t<T>)a);
		return (T)(a / b);
	});
}

void Weaver::append(const Weaver::IMod& ins, size_t lvl) {
	auto& level = this->levels[lvl];
	level.instructions.push_back(ins);

	binaryOp(level.get(ins.dst), level.get(ins.src), "mod", []<typename T>(T a, T b) {
		checkDivisor(b, "mod");
		if (b == -1) return (T)0;
		return (T)(a % b);
	});
}

void Weaver::append(const Weaver::IAnd& ins, size_t lvl) {
	auto& level = this->levels[lvl];
	level.instructions.push_back(ins);

	binaryOp(level.get(ins.dst), level.get(ins.src), "and", []<typename T>(T a, T b) {
		return (T)(a & b);
	});
}

void Weaver::append(const Weaver::IOr& ins, size_t lvl) {
	auto& level = this->levels[lvl];
	level.instructions.push_back(ins);

	binaryOp(level.get(ins.dst), level.get(ins.src), "or", []<typename T>(T a, T b) {
		return (T)(a | b);
	});
}

void Weaver::append(const Weaver::IXor& ins, size_t lvl) {
	auto& level = this->levels[lvl];
	level.instructions.push_back(ins);

	binaryOp(level.get(ins.dst), level.get(ins.src), "xor", []<typename T>(T a, T b) {
		return (T)(a ^ b);
	});
}

// Shifts: a count >= bit width shifts every bit out

void Weaver::append(const Weaver::IShl& ins, size_t lvl) {
	auto& level = this->levels[lvl];
	level.instructions.push_back(ins);

	shiftOp(level.get(ins.dst), level.get(ins.src), "shl", []<typename T>(T a, uint64_t n) {
		if (n >= sizeof(T) * 8) return (T)0;
		return (T)((U_t<T>)a << n);
	});
}

void Weaver::append(const Weaver::IShr& ins, size_t lvl) {
	auto& level = this->levels[lvl];
	level.instructions.push_back(ins);

	// Logical: fills with zeros
	shiftOp(level.get(ins.dst), level.get(ins.src), "shr", []<typename T>(T a, uint64_t n) {
		if (n >= sizeof(T) * 8) return (T)0;
		return (T)((U_t<T>)a >> n);
	});
}

void Weaver::append(const Weaver::ISar& ins, size_t lvl) {
	auto& level = this->levels[lvl];
	level.instructions.push_back(ins);

	// Arithmetic: fills with the sign bit
	shiftOp(level.get(ins.dst), level.get(ins.src), "sar", []<typename T>(T a, uint64_t n) {
		if (n >= sizeof(T) * 8) return (T)(a < 0 ? -1 : 0);
		return (T)(a >> n);
	});
}

void Weaver::append(const Weaver::INot& ins, size_t lvl) {
	auto& level = this->levels[lvl];
	level.instructions.push_back(ins);

	unaryOp(level.get(ins.dst), "not", []<typename T>(T a) {
		return (T)~a;
	});
}

void Weaver::append(const Weaver::INeg& ins, size_t lvl) {
	auto& level = this->levels[lvl];
	level.instructions.push_back(ins);

	unaryOp(level.get(ins.dst), "neg", []<typename T>(T a) {
		return (T)((U_t<T>)0 - (U_t<T>)a);
	});
}

void Weaver::append(const Weaver::IInc& ins, size_t lvl) {
	auto& level = this->levels[lvl];
	level.instructions.push_back(ins);

	unaryOp(level.get(ins.dst), "inc", []<typename T>(T a) {
		return (T)((U_t<T>)a + 1);
	});
}

void Weaver::append(const Weaver::IDec& ins, size_t lvl) {
	auto& level = this->levels[lvl];
	level.instructions.push_back(ins);

	unaryOp(level.get(ins.dst), "dec", []<typename T>(T a) {
		return (T)((U_t<T>)a - 1);
	});
}
