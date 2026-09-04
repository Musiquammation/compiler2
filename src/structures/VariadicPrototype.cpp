#include "VariadicPrototype.hpp"
#include "StructureError.hpp"
#include "symbol/ClassSymbol.hpp"

static StructureError errorTodo(const char* message) {
	return StructureError{
		"Todo",
		message
	};
}


ClassSymbol VariadicPrototype::getMinimalClass() const {
	return ClassSymbol::NONE;
}

std::vector<ClassMember*> VariadicPrototype::followClassMembers(label_t* array, size_t length) const {
    throw errorTodo("VariadicPrototype::followClassMembers");
}