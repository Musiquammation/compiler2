#include "Class.hpp"
#include "ClassMember.hpp"
#include "Logger.hpp"
#include "RealPrototype.hpp"
#include "VariadicPrototype.hpp"
#include "primitives.hpp"
#include <cstdio>

int main() {
	auto logger = getLogger("main");

	logger.debug("value %d", getPrimitive("int32")->primitive->size);
	logger.debug("value %d", getPrimitive("int")->primitive->size);

	return 0;
}
