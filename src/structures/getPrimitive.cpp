#include "Class.hpp"
#include "Primitive.hpp"
#include "RealPrototype.hpp"

#include <array>
#include <cstddef>
#include <cstdio>
#include <memory>
#include <string_view>
#include <unordered_map>

struct PrimitiveDefinition {
	int size;
	bool isSigned;
	bool isFloat;
	const char* name;
};

static constexpr std::array<PrimitiveDefinition, 10> primitiveDefinitions{{
	{1,  true,  false, "int8"},
	{1,  false, false, "uint8"},
	{2,  true,  false, "int16"},
	{2, false, false, "uint16"},
	{4,  true,  false, "int32"},
	{4, false, false, "uint32"},
	{8,  true,  false, "int64"},
	{8, false, false, "uint64"},
	{4,  true,  true,  "float32"},
	{8,  true,  true,  "float64"},
}};

static constexpr std::array<std::pair<std::string_view, int>, 20> primitiveAliases{{
	{"char",   0},
	{"byte",   1},
	{"short",  2},
	{"ushort", 3},
	{"int",    4},
	{"uint",   5},
	{"long",   6},
	{"ulong",  7},
	{"float",  8},
	{"double", 9},
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

static const auto primitiveProtos = [] {
	std::array<std::shared_ptr<RealPrototype>, primitiveDefinitions.size()> result{};

	for (std::size_t i = 0; i < primitiveDefinitions.size(); ++i) {
		result[i] = std::shared_ptr<RealPrototype>(
			RealPrototype::create({&classes[i]}, {})
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

	for (const auto& [alias, index] : primitiveAliases) {
		result.emplace(alias, &classes[index]);
	}

	return result;
}();

static const std::unordered_map<std::string_view, std::shared_ptr<RealPrototype>> primitiveProtoMap = [] {
	std::unordered_map<std::string_view, std::shared_ptr<RealPrototype>> result;

	for (std::size_t i = 0; i < primitiveDefinitions.size(); ++i) {
		result.emplace(
			primitiveDefinitions[i].name,
			primitiveProtos[i]
		);
	}

	for (const auto& [alias, index] : primitiveAliases) {
		result.emplace(alias, primitiveProtos[index]);
	}

	return result;
}();

const Class* getPrimitive(label_t label) {
	auto it = primitiveMap.find(label);

	if (it == primitiveMap.end())
		return nullptr;

	return it->second;
}

std::shared_ptr<RealPrototype> getPrimitiveProto(label_t label) {
	auto it = primitiveProtoMap.find(label);

	if (it == primitiveProtoMap.end())
		return nullptr;

	return it->second;
}