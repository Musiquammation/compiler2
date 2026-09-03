#include "structures/Class.hpp"
#include "structures/ClassMember.hpp"
#include "structures/RealPrototype.hpp"
#include "util/Logger.hpp"
#include <cstdio>
#include <vector>
#include "structures/getPrimitive.hpp"

int main() {
	auto logger = getLogger("main");

	auto proto_int = getPrimitiveProto("int");
	auto proto_short = getPrimitiveProto("short");

	std::vector<ClassMemberBuilder> cl_Point_members = {
		ClassMemberBuilder{.name = "x", .proto = {proto_int}},	
		ClassMemberBuilder{.name = "y", .proto = {proto_int}},
	};

	auto cl_Point = Class::create("Point", nullptr, cl_Point_members);

	std::vector<ClassMemberBuilder> cl_FlatRect3D_members = {
		ClassMemberBuilder{.name = "start", .proto = {RealPrototype::create({&cl_Point}, {})}},	
		ClassMemberBuilder{.name = "end", .proto = {RealPrototype::create({&cl_Point}, {})}},
		ClassMemberBuilder{.name = "z", .proto = {proto_int}},
	};

	auto cl_FlatRect3D = Class::create("FlatRect3D", nullptr, cl_FlatRect3D_members);

	logger.info("FlatRect3d %d %d", cl_FlatRect3D.size.size, cl_FlatRect3D.size.align);

	


	return 0;
}
