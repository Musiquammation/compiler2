#include "RealPrototype.hpp"
#include "Class.hpp"
#include <memory>

std::shared_ptr<RealPrototype> RealPrototype::create(
    const Class* clas,
    std::map<VariadicPrototype*, Prototype> settings
) {
    auto result = std::shared_ptr<RealPrototype>(
        new RealPrototype()
    );

    result->clas = clas;
    result->settings = std::move(settings);

    if (clas->size.isValid()) {
        result->size = clas->size;
    } else {
        /// TODO: try to eval size
        result->size = clas->size;
    }

    return result;
}
