#include "structures/Class.hpp"
#include "structures/ClassMember.hpp"
#include "structures/Prototype.hpp"
#include "structures/RealPrototype.hpp"
#include "structures/Variable.hpp"
#include "structures/VariadicPrototype.hpp"
#include "util/Logger.hpp"
#include <cstdio>
#include <memory>
#include <vector>
#include "structures/getPrimitive.hpp"

int main() {
	struct {
		std::unique_ptr<Class> Point;
		std::unique_ptr<Class> FlatRect3D;
		std::unique_ptr<Class> Type;
		std::unique_ptr<Class> Pair;
		std::unique_ptr<Class> IntPairWrapper;
		std::unique_ptr<Class> Wrapper;
	} cl;

	auto logger = getLogger("main");

	auto proto_int = getPrimitiveProto("int");
	auto proto_short = getPrimitiveProto("short");
	auto proto_size_t = getPrimitiveProto("size_t");

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

	// Type
	{
		std::vector<ClassMemberBuilder> members = {
			{.name = "addr", .proto = Prototype{proto_size_t}},
		};

		ClassBuilder builder = {"Type", std::move(members), nullptr};
		cl.Type = builder.build();
	}

	// Pair
	{
		std::vector<ClassMemberBuilder> metaMembers = {{
			.name = "T",
			.proto = Prototype{RealPrototype::create({cl.Type.get()}, {})}
		},};

		ClassBuilder metaBuilder = {nullptr, std::move(metaMembers), nullptr};

		/// TODO: fix that
		auto proto = VariadicPrototypeBuilder{"T"};

		std::vector<ClassMemberBuilder> members = {
			{.name = "first", .proto = proto},
			{.name = "choosen", .proto = Prototype{proto_short}},
			{.name = "second", .proto = proto},
		};

		ClassBuilder builder = {"Pair", std::move(metaMembers), &metaBuilder};
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
			{.name = "Wrapped", .proto = Prototype{RealPrototype::create({cl.Type.get()}, {})}},
		};
		ClassBuilder metaBuilder = {nullptr, std::move(metaMembers), nullptr};

		/// TODO: fix that
		auto proto = VariadicPrototypeBuilder{"Wrapped"};

		std::vector<ClassMemberBuilder> members = {
			{.name = "wrap", .proto = {proto}},
		};

		ClassBuilder builder = {"Wrapped", std::move(members), &metaBuilder};
		cl.Wrapper = builder.build();
	}

	// TODO: followClassMembers with size
	// test followClassMembers
	{
		auto rp = RealPrototype::create({cl.Wrapper.get()}, {
			{cl.Wrapper->meta->members.search("Wrapped"), {RealPrototype::create({cl.IntPairWrapper.get()}, {})}}
		});
		
		auto v = Variable {
			"wrapper",
			Prototype{rp},
			nullptr
		};

		
		label_t array[] = {"wrap", "content", "second", "start", "y"};
		auto m = v.getPlacement(array, sizeof(array)/sizeof(label_t));
		logger.debug("%d %d +%d", m.size.size, m.size.align, m.offset);
		


		/*
		auto f = v.proto.followClassMembers(array, sizeof(array)/sizeof(label_t));
		if (f.has_value()) {
			logger.debug(
				"%s %d | %d %d %d",
				f->proto.getMinimalClass().getSymbol(),
				f->members.size(),
				f->placement.size.size,
				f->placement.size.align,
				f->placement.offset
			);
		} else {
			logger.debug("Failed");
		}
			*/
	}




	return 0;
}
