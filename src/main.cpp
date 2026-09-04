#include "structures/Class.hpp"
#include "structures/ClassMember.hpp"
#include "structures/RealPrototype.hpp"
#include "util/Logger.hpp"
#include <cstdio>
#include <vector>
#include "structures/getPrimitive.hpp"

int main() {
	struct {
		Class Point;
		Class FlatRect3D;
		Class Type;
		Class Pair_meta;
		Class Pair;
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

		logger.info(
			"FlatRect3d %d %d",
			cl.FlatRect3D.size.size,
			cl.FlatRect3D.size.align
		);
	}

	// Pair_meta
	{

	}


	
	// followClassMembers
	{
		auto proto = RealPrototype::create({&cl.FlatRect3D}, {});
		label_t path[] = {"end", "x"};
		auto members = proto->followClassMembers(path, sizeof(path)/sizeof(label_t));
		logger.debug("Members:");
		for (auto i: members) {
			logger.debug("\t %s %2d", i->name, i->placement.offset);
		}


	}
	


	return 0;
}
