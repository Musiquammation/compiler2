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
	logger.info("%d %d", cl_Point.size.size, cl_Point.size.align);

	return 0;
}
