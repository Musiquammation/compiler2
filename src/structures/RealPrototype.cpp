#include "RealPrototype.hpp"
#include "Class.hpp"
#include "symbol/ClassSymbol.hpp"
#include <memory>

std::shared_ptr<RealPrototype> RealPrototype::create(
	ClassSymbol clas,
	std::map<const ClassMember*, Prototype> settings
) {
	auto result = std::shared_ptr<RealPrototype>(
		new RealPrototype()
	);

	result->clas = clas;
	result->settings = std::move(settings);

	auto cl = clas.get();

	if (cl->size.isValid()) {
		result->size = cl->size;
	} else {
		/// TODO: try to eval size
		result->size = cl->size;
	}

	return result;
}


std::optional<Prototype> RealPrototype::searchVariadic(const ClassMember* member) const {
	auto it = this->settings.find(member);

	return it != this->settings.end()
		? std::optional<Prototype>{it->second}
		: std::nullopt;
}

ClassSymbol RealPrototype::getMinimalClass() const {
	return this->clas;
}


std::vector<ClassMember*> RealPrototype::followClassMembers(label_t* array, size_t length) const {
	std::vector<ClassMember*> members;

	if (length == 0)
		return members;

	auto currentMember = this->clas.force()->members.search(array[0]);
	
	if (currentMember == nullptr)
		return members;

	members.push_back(currentMember);

	for (size_t i = 1; i < length; i++) {
		auto cl = currentMember->proto.getMinimalClass();
		auto next = cl.force()->members.search(array[i]);
		if (next == nullptr)
			return members;
		
		currentMember = next;
		members.push_back(currentMember);
	}
	

	return members;
}


