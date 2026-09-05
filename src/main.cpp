#include "structures/Class.hpp"
#include "structures/ClassMember.hpp"
#include "structures/Prototype.hpp"
#include "structures/RealPrototype.hpp"
#include "structures/VariadicPrototype.hpp"
#include "util/Logger.hpp"
#include <cstdio>
#include <memory>
#include <vector>
#include "structures/getPrimitive.hpp"

int main() {
	struct {
		Class Point;
		Class FlatRect3D;
		Class Type;
		Class Pair;
		Class IntPairWrapper;
	} cl;

	auto logger = getLogger("main");

	auto proto_int = getPrimitiveProto("int");
	auto proto_short = getPrimitiveProto("short");
	auto proto_size_t = getPrimitiveProto("size_t");

	// Point
	{
		std::vector<ClassMemberBuilder> members = {
			{.name = "x", .proto = {proto_int}},
			{.name = "y", .proto = {proto_int}},
		};

		cl.Point = Class::create("Point", nullptr, members);
	}

	// FlatRect3D
	{
		std::vector<ClassMemberBuilder> members = {
			{
				.name = "start",
				.proto = {
					RealPrototype::create({&cl.Point}, {})
				}
			},
			{
				.name = "z",
				.proto = {proto_short}
			},
			{
				.name = "end",
				.proto = {
					RealPrototype::create({&cl.Point}, {})
				}
			},
		};

		cl.FlatRect3D = Class::create("FlatRect3D", nullptr, members);

		logger.debug(
			"FlatRect3d %d %d",
			cl.FlatRect3D.size.size,
			cl.FlatRect3D.size.align
		);
	}

	// Type
	{
		std::vector<ClassMemberBuilder> members = {
			{.name = "addr", .proto = {proto_size_t}},
		};

		cl.Type = Class::create(nullptr, nullptr, members);
	}

	// Pair
	{
		std::vector<ClassMemberBuilder> metaMembers = {
			{.name = "T", .proto = {RealPrototype::create({&cl.Type}, {})}},
		};

		auto Pair_meta = std::make_unique<Class>(
			Class::create(nullptr, nullptr, metaMembers)
		);

		auto proto = VariadicPrototype::create(
			Pair_meta->members.search("T")
		);

		std::vector<ClassMemberBuilder> members = {
			{.name = "mb0", .proto = {proto}},
			{.name = "mb1", .proto = {proto}}
		};

		cl.Pair = Class::create(nullptr, std::move(Pair_meta), members);
	}

	
	// followClassMembers
	{
		auto proto = RealPrototype::create({&cl.FlatRect3D}, {});
		label_t path[] = {"end", "x"};
		auto members = proto->followClassMembers(path, sizeof(path)/sizeof(label_t));
		logger.debug("followClassMembers");
		for (auto i: members) {
			logger.debug("\t %s %2d", i->name, i->placement.offset);
		}
	}

	// IntPairWrapper
	{
		auto T = cl.Pair.meta->members.search("T");
		auto proto = RealPrototype::create(
			{&cl.Pair},
			{
				{T, {proto_int}}
			}
		);

		std::vector<ClassMemberBuilder> members = {
			{.name = "content", .proto = {proto}},
		};

		cl.IntPairWrapper = Class::create(nullptr, nullptr, members);
		logger.debug("IntPairWrapper size=%d", cl.IntPairWrapper.size.size);
	}


	


	return 0;
}
