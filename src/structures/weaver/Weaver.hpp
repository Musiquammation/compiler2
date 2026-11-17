#pragma once

#include "../../util/TracedSortedArray.hpp"
#include <cstddef>
#include <fstream>
#include <variant>
#include <vector>

struct Function;
struct Prototype;
struct Argument;
class Variable;

class Weaver {
	using arg_t = uint64_t;
	using var_t = size_t;

	static constexpr var_t NO_VAR = SIZE_MAX;
	// Tags ids of slipped variables (stored apart from regular ones)
	static constexpr var_t SLIP_BIT = (var_t)1 << (sizeof(var_t) * 8 - 1);


public:
	struct VarDesc {
		var_t variable;
		size_t offset;
	};


	struct IncScope {};
	struct DecScope {};

	struct Create {
		var_t name;
		size_t size;
	};

	struct ISet {
		VarDesc dst;
		arg_t arg;
		size_t size;
	};

	struct ICpy {
		VarDesc dst;
		VarDesc src;
		size_t size;
	};

	struct IAdd {
		var_t dst;
		var_t src;
	};

	struct ISub {
		var_t dst;
		var_t src;
	};

	struct IMul {
		var_t dst;
		var_t src;
	};

	struct IDiv {
		var_t dst;
		var_t src;
	};

	struct IMod {
		var_t dst;
		var_t src;
	};

	struct IAnd {
		var_t dst;
		var_t src;
	};

	struct IOr {
		var_t dst;
		var_t src;
	};

	struct IXor {
		var_t dst;
		var_t src;
	};

	struct INot {
		var_t dst;
	};

	struct INeg {
		var_t dst;
	};

	struct IShl {
		var_t dst;
		var_t src;
	};

	struct IShr {
		var_t dst;
		var_t src;
	};

	struct ISar {
		var_t dst;
		var_t src;
	};

	struct IInc {
		var_t dst;
	};

	struct IDec {
		var_t dst;
	};

	struct ILoad {
		var_t dst;
		var_t src;
		size_t size;
	};

	struct IStore {
		var_t dst;
		var_t src;
		size_t size;
	};

	struct IIf {
		VarDesc src;
	};

	struct IElse {

	};

	struct IWhile {

	};

	struct IBreak {

	};
	
	struct IPass {
		VarDesc dst;
	};

	struct ICall {
		VarDesc dst;
		Function* fn;
	};

	struct IReturn {
		VarDesc src;
	};

	struct ISlip {
		var_t src;
		size_t dst; // arg index
		var_t level;
	};

private:
	using Instruction = std::variant<
		Create,
		IncScope,
		DecScope,
		ISet,
		ICpy,
		IAdd,
		ISub,
		IMul,
		IDiv,
		IMod,
		IAnd,
		IOr,
		IXor,
		INot,
		INeg,
		IShl,
		IShr,
		ISar,
		IInc,
		IDec,
		ILoad,
		IStore,
		IIf,
		IElse,
		IWhile,
		IBreak,
		IPass,
		ICall,
		IReturn,
		ISlip
	>;

	struct Level;
	struct WVariable;
	class Transpiler;

	std::vector<Argument>& args;
	std::vector<Level> levels;
	std::ofstream& stream;
	
	Level& getLevel(size_t level);

public:
	Weaver(std::vector<Argument>& args, std::ofstream& stream);
	~Weaver();

	var_t create(label_t name, Prototype& proto, size_t level, bool createMeta = true);

	var_t search(label_t name, size_t level);
	var_t meta(var_t var, size_t level);
	Variable& variable(var_t var, size_t level);
	
	var_t slip(var_t v, size_t dstLevel, size_t srcLevel);	



	void append(const IncScope& ins, size_t lvl);
	void append(const DecScope& ins, size_t lvl);
	void append(const ISet& ins, size_t lvl);
	void append(const ICpy& ins, size_t lvl);
	void append(const IAdd& ins, size_t lvl);
	void append(const IMul& ins, size_t lvl);
	void append(const ISlip& ins, size_t lvl);

};


