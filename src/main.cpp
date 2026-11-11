#include "structures/Class.hpp"
#include "structures/ClassMember.hpp"
#include "structures/BuildablePrototype.hpp"
#include "structures/Function.hpp"
#include "structures/ProcResPrototype.hpp"
#include "structures/Procedure.hpp"
#include "structures/Prototype.hpp"
#include "structures/RealPrototype.hpp"
#include "structures/StructureError.hpp"
#include "structures/Variable.hpp"
#include "structures/weaver/Weaver.hpp"
#include "util/Logger.hpp"
#include <memory>
#include <vector>
#include "structures/getPrimitive.hpp"


void runTest(Logger& logger) {
	struct {
		std::unique_ptr<Class>
			Variadic,
			Type,
			Pointer,
			Point,
			FlatRect3D,
			Pair,
			Integer,
			IntPairWrapper,
			Wrapper
			;
	} cl;


	auto proto_int = getPrimitiveProto("int");
	auto proto_short = getPrimitiveProto("short");
	auto proto_byte = getPrimitiveProto("byte");
	auto proto_size_t = getPrimitiveProto("size_t");


	// STD CLASSES
	// Variadic
	{
		std::vector<ClassMemberBuilder> members = {};

		ClassBuilder builder = {"Variadic", std::move(members), nullptr};
		cl.Variadic = builder.build();
	}

	// Type
	{
		std::vector<ClassMemberBuilder> metaMembers = {
			{
				.name = "V",
				.proto = Prototype{RealPrototype::create({cl.Variadic.get()}, {})}
			},
		};

		std::vector<ClassMemberBuilder> members = {
			{.name = "addr", .proto = Prototype{proto_size_t}},
		};

		ClassBuilder metaBuilder = {nullptr, std::move(metaMembers), nullptr};

		ClassBuilder builder = {"Type", std::move(members), &metaBuilder};
		cl.Type = builder.build();
	}

	// Pointer
	{
		auto V = cl.Type->meta->members.search("V");
		auto proto = ProcResPrototypeBuilder::friendVariadic("T");

		std::vector<ClassMemberBuilder> metaMembers = {
			{
				.name = "T",
				.proto = Prototype{RealPrototype::create({cl.Variadic.get()}, {})}
			},
			{
				.name = "type",
				.proto = RealPrototypeBuilder{
					{cl.Type.get()},
					{
						{V, proto}
					}
				}
			},
			{.name = "readOk", .proto = Prototype{proto_byte}},
			{.name = "writeOk", .proto = Prototype{proto_byte}},
		};

		ClassBuilder metaBuilder = {nullptr, std::move(metaMembers), nullptr};

		std::vector<ClassMemberBuilder> members = {
			{.name = "addr", .proto = Prototype{proto_size_t}},
		};

		ClassBuilder builder = {"Pointer", std::move(members), &metaBuilder};
		cl.Pointer = builder.build();
	}
	



	// TEST CLASSES


	// Point
	{
		std::vector<ClassMemberBuilder> members = {
			{.name = "x", .proto = Prototype{proto_int}},
			{.name = "y", .proto = Prototype{proto_int}},
		};

		ClassBuilder builder = {"Point", std::move(members), nullptr};
		cl.Point = builder.build();
	}

	// FlatRect3D
	{
		std::vector<ClassMemberBuilder> members = {
			{
				.name = "start",
				.proto = Prototype{
					RealPrototype::create({cl.Point.get()}, {})
				}
			},
			{
				.name = "z",
				.proto = Prototype{proto_short}
			},
			{
				.name = "end",
				.proto = Prototype{
					{RealPrototype::create({cl.Point.get()}, {})}
				}
			},
		};


		ClassBuilder builder = {
			"FlatRect3D", 
			std::move(members), 
			nullptr
		};
		cl.FlatRect3D = builder.build();

		logger.debug(
			"FlatRect3d %d %d",
			cl.FlatRect3D->size.size,
			cl.FlatRect3D->size.align
		);
	}


	// Pair
	{
		std::vector<ClassMemberBuilder> metaMembers = {
			{
				.name = "T",
				.proto = Prototype{RealPrototype::create({cl.Variadic.get()}, {})}
			},
			{
				.name = "rect",
				.proto = Prototype{RealPrototype::create({cl.FlatRect3D.get()}, {})}
			},
		};

		ClassBuilder metaBuilder = {nullptr, std::move(metaMembers), nullptr};

		auto proto = ProcResPrototypeBuilder::variadic("T");

		std::vector<ClassMemberBuilder> members = {
			{.name = "first", .proto = proto},
			{.name = "choosen", .proto = Prototype{proto_short}},
			{.name = "second", .proto = proto},
		};

		ClassBuilder builder = {"Pair", std::move(members), &metaBuilder};
		cl.Pair = builder.build();
		cl.Pair->describe(logger);
	}

	// Integer
	{
		std::vector<ClassMemberBuilder> metaMembers = {
			{
				.name = "value",
				.proto = Prototype{proto_int}
			},
			{
				.name = "valueIsKnown",
				.proto = Prototype{proto_byte}
			},
			{
				.name = "sign",
				.proto = Prototype{proto_short}
			},
		};

		ClassBuilder metaBuilder = {nullptr, std::move(metaMembers), nullptr};

		std::vector<ClassMemberBuilder> members = {
			{.name = "number", .proto = Prototype{proto_int}}
		};

		ClassBuilder builder = {"Integer", std::move(members), &metaBuilder};
		cl.Integer = builder.build();
		cl.Integer->describe(logger);
	}
	

	// followClassMembers
	{
		auto proto = RealPrototype::create({cl.FlatRect3D.get()}, {});
		label_t path[] = {"end", "x"};
		auto members = proto->followClassMembers(path, sizeof(path)/sizeof(label_t));
		logger.debug("followClassMembers");
		auto _ = logger.tabulator();
		for (auto i: members) {
			logger.debug("%s %2d", i->name, i->placement.offset);
		}
	}

	// IntPairWrapper
	{
		auto T = cl.Pair->meta->members.search("T");
		auto proto = RealPrototype::create(
			{cl.Pair.get()},
			std::map<const ClassMember*, Prototype>{
				{
					T,
					Prototype{RealPrototype::create({cl.FlatRect3D.get()}, {})}
				}
			}
		);

		std::vector<ClassMemberBuilder> members = {
			{.name = "pad0", .proto = Prototype{proto_short}},
			{.name = "content", .proto = Prototype{proto}},
			{.name = "pad1", .proto = Prototype{RealPrototype::create({cl.Point.get()}, {})}},
		};

		ClassBuilder builder = {"IntPairWrapper", std::move(members), nullptr};
		cl.IntPairWrapper = builder.build();
		logger.debug("IntPairWrapper size=%d isEntire=%d", cl.IntPairWrapper->size.size, cl.IntPairWrapper->isEntire());
		cl.IntPairWrapper->describe(logger);
	}

	// Wrapper
	{
		std::vector<ClassMemberBuilder> metaMembers = {
			{
				.name = "Wrapped",
				.proto = Prototype{RealPrototype::create({cl.Variadic.get()}, {})}
			},
			{
				.name = "compiledPoint",
				.proto = Prototype{proto_int}
			}
		};
		ClassBuilder metaBuilder = {nullptr, std::move(metaMembers), nullptr};

		// !IL MANQUE WRAP DANS LA META!
		auto proto = ProcResPrototypeBuilder::variadic("Wrapped");

		std::vector<ClassMemberBuilder> members = {
			{.name = "wrap", .proto = {proto}},
		};

		ClassBuilder builder = {"Wrapper", std::move(members), &metaBuilder};
		logger.debug("[[[Building Wrapper]]]");

		cl.Wrapper = builder.build();
		cl.Wrapper->describe(logger);
	}

	{
		auto wp = RealPrototype::create(
			{cl.Wrapper.get()},
			{
				{
					cl.Wrapper->meta->members.search("Wrapped"),
					{
						RealPrototype::create(
							{cl.Pair.get()},
							{
								{
									cl.Pair->meta->members.search("T"),
									{
										RealPrototype::create(
										{cl.IntPairWrapper.get()},
										{}
										)
									}
								}
							}
						)
					}
				}
			}
		);

		auto v = Variable{
			"wrapper",
			Prototype{wp}
		};

		
		label_t array[] = {"wrap"};
		auto m = v.getPlacement(array, sizeof(array)/sizeof(label_t));
		logger.debug("%d %d @%+d", m.size.size, m.size.align, m.offset);
	}



	// function test
	{
		auto fn = Function{
			.args = {},
			.ret = {ReturnType::Void{}},
			.cname = "test"
		};

		std::ofstream stream{"out/test.c"};

		Weaver weaver{fn.args, stream};

		Prototype proto_Integer {RealPrototype::create({cl.Integer.get()}, {})};
		auto a = weaver.create("a", proto_Integer, 0);
		auto var_a = weaver.variable(a, 0);

		auto meta_a = weaver.meta(a, 0);
		auto var_meta_a = weaver.variable(meta_a, 1);

		// Set a::value to 0
		{
			label_t members[] = {"value"};
			auto placement = var_meta_a.getPlacement(members, 1);
			weaver.append(Weaver::ISet{
				.dst = {.variable = meta_a, .offset = (size_t)placement.offset},
				.arg = 0,
				.size = (size_t)placement.size.size
			}, 1);
		}

		// Set a::valueIsKnown to 0
		{
			label_t members[] = {"valueIsKnown"};
			auto placement = var_meta_a.getPlacement(members, 1);
			weaver.append(Weaver::ISet{
				.dst = {.variable = meta_a, .offset = (size_t)placement.offset},
				.arg = 0,
				.size = (size_t)placement.size.size
			}, 1);
		}

		// Set a::sign to 0
		{
			label_t members[] = {"sign"};
			auto placement = var_meta_a.getPlacement(members, 1);
			weaver.append(Weaver::ISet{
				.dst = {.variable = meta_a, .offset = (size_t)placement.offset},
				.arg = 0,
				.size = (size_t)placement.size.size
			}, 1);
		}

		// Set a.number to 5
		/*{
			label_t members[] = {"number"};
			auto placement = var_a.getPlacement(members, 1);
			weaver.append(Weaver::ISet{
				.dst = {.variable = a, .offset = (size_t)placement.offset},
				.arg = 0,
				.size = (size_t)placement.size.size
			}, 0);
		}*/


		// Construct b = Integer{10}
		auto b = weaver.create("b", proto_Integer, 1);
		auto var_b = weaver.variable(b, 1);

		auto meta_b = weaver.meta(b, 1);
		auto var_meta_b = weaver.variable(meta_b, 2);


		{
			// Set b::value to 10
			{
				label_t members[] = {"value"};
				auto placement = var_meta_b.getPlacement(members, 1);
				weaver.append(Weaver::ISet{
					.dst = {.variable = meta_b, .offset = (size_t)placement.offset},
					.arg = 10,
					.size = (size_t)placement.size.size
				}, 2);
			}
			
			// Set b::valueIsKnown to 1
			{
				label_t members[] = {"valueIsKnown"};
				auto placement = var_meta_b.getPlacement(members, 1);
				weaver.append(Weaver::ISet{
					.dst = {.variable = meta_b, .offset = (size_t)placement.offset},
					.arg = 1,
					.size = (size_t)placement.size.size
				}, 2);
			}
			// Set b::sign to 1
			{
				label_t members[] = {"sign"};
				auto placement = var_meta_b.getPlacement(members, 1);
				weaver.append(Weaver::ISet{
					.dst = {.variable = meta_b, .offset = (size_t)placement.offset},
					.arg = 1,
					.size = (size_t)placement.size.size
				}, 2);
			}

			// Set b.number to 10
			{
				label_t members[] = {"number"};
				auto placement = var_b.getPlacement(members, 1);
				weaver.append(Weaver::ISet{
					.dst = {.variable = b, .offset = (size_t)placement.offset},
					.arg = 10,
					.size = (size_t)placement.size.size
				}, 1);
			}
		}
	}


	logger.debug("TEST FINISHED");
}


int main() {
	auto logger = getLogger("main");

	try {
		runTest(logger);
	} catch (const StructureError& e) {
		logger.error(e.what());
		e.printBacktrace();
		return 1;
	}



	return 0;
}