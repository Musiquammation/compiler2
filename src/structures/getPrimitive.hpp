#pragma once

#include "../label_t.hpp"
#include <memory>

struct Class;
struct RealPrototype;

const Class* getPrimitive(label_t label);
const std::shared_ptr<RealPrototype> getPrimitiveProto(label_t label);


