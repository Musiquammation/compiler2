#include "structures/Class.hpp"
#include "util/Logger.hpp"
#include <cstdio>
#include "structures/getPrimitive.hpp"

int main() {
	auto logger = getLogger("main");

	logger.debug("value %d", getPrimitive("int32")->primitive->size);
	logger.debug("value %d", getPrimitive("int")->primitive->size);

	return 0;
}
