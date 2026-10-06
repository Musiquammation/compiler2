#include "structures/Class.hpp"
#include "structures/ClassMember.hpp"
#include "structures/ProcResPrototype.hpp"
#include "structures/Prototype.hpp"
#include "structures/RealPrototype.hpp"
#include "structures/StructureError.hpp"
#include "util/Logger.hpp"
#include <cstdio>
#include <memory>
#include <vector>
#include "structures/getPrimitive.hpp"


void runTest(Logger& logger) {
	struct {
		std::unique_ptr<Class>
			Type,
			Member,
			Reveal,
			Point,
			FlatRect3D,
			Pair,
			IntPairWrapper,
			Wrapper
			;
	} cl;


	auto proto_int = getPrimitiveProto("int");
	auto proto_short = getPrimitiveProto("short");
	auto proto_size_t = getPrimitiveProto("size_t");


	// STD CLASSES
	// Type
	{
		std::vector<ClassMemberBuilder> members = {
			{.name = "addr", .proto = Prototype{proto_size_t}},
		};

		ClassBuilder builder = {"Type", std::move(members), nullptr};
		cl.Type = builder.build();
	}

	// Member
	{
		std::vector<ClassMemberBuilder> metaMembers = {{
			.name = "M",
			.proto = Prototype{RealPrototype::create({cl.Type.get()}, {})}
		}};
		ClassBuilder metaBuilder = {nullptr, std::move(metaMembers), nullptr};

		std::vector<ClassMemberBuilder> members = {
			{.name = "addr", .proto = Prototype{proto_size_t}},
		};

		ClassBuilder builder = {"Member", std::move(members), &metaBuilder};
		cl.Member = builder.build();
	}

	// Reveal






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

		ClassBuilder builder = {"FlatRect3D", std::move(members), nullptr};
		cl.FlatRect3D = builder.build();

		logger.debug(
			"FlatRect3d %d %d",
			cl.FlatRect3D->size.size,
			cl.FlatRect3D->size.align
		);
	}


	// Pair
	{
		std::vector<ClassMemberBuilder> metaMembers = {{
			.name = "T",
			.proto = Prototype{RealPrototype::create({cl.Type.get()}, {})}
		}};

		ClassBuilder metaBuilder = {nullptr, std::move(metaMembers), nullptr};

		auto proto = ProcResPrototypeBuilder::variadic("T");

		std::vector<ClassMemberBuilder> members = {
			{.name = "first", .proto = proto},
			{.name = "choosen", .proto = Prototype{proto_short}},
			{.name = "second", .proto = proto},
		};

		ClassBuilder builder = {"Pair", std::move(members), &metaBuilder};
		cl.Pair = builder.build();
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
			{
				{
					T,
					{RealPrototype::create({cl.FlatRect3D.get()}, {})}
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
	}

	// Wrapper
	{
		std::vector<ClassMemberBuilder> metaMembers = {
			{
				.name = "Wrapped",
				.proto = Prototype{RealPrototype::create({cl.Type.get()}, {})}
			},
			{
				.name = "compiledPoint",
				.proto = Prototype{
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
		};
		ClassBuilder metaBuilder = {nullptr, std::move(metaMembers), nullptr};

		auto proto = ProcResPrototypeBuilder::variadic("Wrapped");

		std::vector<ClassMemberBuilder> members = {
			{.name = "wrap", .proto = {proto}},
		};

		ClassBuilder builder = {"Wrapper", std::move(members), &metaBuilder};
		cl.Wrapper = builder.build();
	}

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