#include "VariadicPrototype.hpp"
#include "StructureError.hpp"
#include "Class.hpp"
#include "symbol/ClassSymbol.hpp"
#include <format>
#include <memory>

static StructureError errorTodo(const char* message) {
	return StructureError{
		"Todo",
		message
	};
}


ClassSymbol VariadicPrototype::getMinimalClass() const {
	if (this->minimal.has_value())
		return this->minimal->getMinimalClass();

	return ClassSymbol::NONE;
}

std::vector<ClassMember*> VariadicPrototype::followClassMembers(label_t* array, size_t length) const {
	throw errorTodo("VariadicPrototype::followClassMembers");
}



std::shared_ptr<VariadicPrototype> VariadicPrototype::create(
	ClassMember* reference,
	Prototype minimal
) {
	return std::make_shared<VariadicPrototype>(reference, minimal);
}

std::shared_ptr<VariadicPrototype> VariadicPrototype::create(ClassMember* reference) {
	return std::make_shared<VariadicPrototype>(reference);
}


std::shared_ptr<VariadicPrototype> VariadicPrototypeBuilder::build(const Class* source) const {
	if (source == nullptr) {
		throw StructureError{
			"VariadicCreationNullScope",
			std::format(
				"Tried to search variadic member {} in null class",
				this->name
			)
		};
	}

	auto ref = source->members.search(this->name);
	if (ref == nullptr) {
		throw StructureError{
			"VariadicReferenceNotResolved",
			std::format(
				"Could not find variadic member {} inside {}",
				this->name,
				source->name
			)
		};
	}

	return VariadicPrototype::create(ref);
}