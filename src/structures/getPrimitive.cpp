#include "Class.hpp"
#include "Primitive.hpp"

#include <array>
#include <cstddef>
#include <string_view>
#include <unordered_map>

struct PrimitiveDefinition {
	int size;
	bool isSigned;
	bool isFloat;
	const char* name;
};

static constexpr std::array<PrimitiveDefinition, 10> primitiveDefinitions{{
	{8,  true,  false, "int8"},
	{8,  false, false, "uint8"},
	{16, true,  false, "int16"},
	{16, false, false, "uint16"},
	{32, true,  false, "int32"},
	{32, false, false, "uint32"},
	{64, true,  false, "int64"},
	{64, false, false, "uint64"},
	{32, true,  true,  "float32"},
	{64, true,  true,  "float64"},
}};

static constexpr std::array<std::pair<std::string_view, int>, 20> primitiveAliases{{
	{"char",   0}, // int8
	{"byte",   1}, // uint8
	{"short",  2}, // int16
	{"ushort", 3}, // uint16
	{"int",    4}, // int32
	{"uint",   5}, // uint32
	{"long",   6}, // int64
	{"ulong",  7}, // uint64
	{"float",  8}, // float32
	{"double", 9}, // float64
}};

static const auto primitives = [] {
	std::array<Primitive, primitiveDefinitions.size()> result{};

	for (std::size_t i = 0; i < primitiveDefinitions.size(); ++i) {
		const auto& definition = primitiveDefinitions[i];

		result[i] = {
			definition.size,
			definition.isSigned,
			definition.isFloat
		};
	}

	return result;
}();

static const auto classes = [] {
	std::array<Class, primitiveDefinitions.size()> result{};

	for (std::size_t i = 0; i < primitiveDefinitions.size(); ++i) {
		result[i] = Class::createPrimitive(
			&primitives[i],
			primitiveDefinitions[i].name
		);
	}

	return result;
}();

static const std::unordered_map<std::string_view, const Class*> primitiveMap = [] {
	std::unordered_map<std::string_view, const Class*> result;

	for (std::size_t i = 0; i < primitiveDefinitions.size(); ++i) {
		result.emplace(
			primitiveDefinitions[i].name,
			&classes[i]
		);
	}

	// Add aliases.
	for (const auto& [alias, index] : primitiveAliases) {
		result.emplace(alias, &classes[index]);
	}

	return result;
}();

const Class* getPrimitive(label_t label) {
	auto it = primitiveMap.find(label);

	if (it == primitiveMap.end())
		return nullptr;

	return it->second;
}
