#include "RealPrototype.hpp"
#include "Class.hpp"
#include "symbol/ClassSymbol.hpp"
#include <memory>

std::shared_ptr<RealPrototype> RealPrototype::create(
    ClassSymbol clas,
    std::map<VariadicPrototype*, Prototype> settings
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
