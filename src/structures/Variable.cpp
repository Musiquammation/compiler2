#include "Variable.hpp"

#include "../util/loopPtr.hpp"
#include "../util/MemoryPlacer.hpp"
#include "../util/Logger.hpp"
#include "Class.hpp"
#include "ClassMember.hpp"
#include "Prototype.hpp"
#include "RealPrototype.hpp"
#include "StructureError.hpp"
#include "VariadicPrototype.hpp"
#include "symbol/ClassSymbol.hpp"
#include <cstddef>
#include <format>
#include <map>
#include <memory>

static constexpr int UNPROCESSABLE_OFFSET = -1;
static constexpr int WAITING_OFFSET = -2;

static auto logger = getLogger("variable");

struct Variable::PositionNode {
	const Class* cl;
	MemoryPlacement placement;
	std::unique_ptr<PositionNode[]> children;
};

using PositionNode = Variable::PositionNode;




static void fillNode(
	PositionNode* node,
	const std::map<const ClassMember*, Prototype>& settings,
	std::shared_ptr<RealPrototype> proto
) {
	MemoryPlacer placer;
	auto& members = proto->clas.force()->members;
	auto children = std::make_unique_for_overwrite<PositionNode[]>(members.getLength());
	logger.debug("Allocate %p %d", children.get(), (int)members.getLength());
	auto child = &children[0];

	for (
		auto it = members.trace_cbegin();
		it != members.trace_cend();
		it++, child++
	) {
		ClassMember* m = *it;
		if (m->proto.isEntire()) {
			child->children = nullptr;
			logger.debug(
				"fillNode   trivially '%s' (of '%s')",
				m->name,
				m->proto.getMinimalClass().getSymbol()
			);
			auto _ = logger.tabulator();
			auto size = m->proto.getQuickSize();
			child->cl = m->proto.forceClass();
			child->placement.size = size;
			child->placement.offset = placer.add(size);
			logger.debug(
				"Place '%s' at +%d",
				m->name,
				child->placement.offset
			);
			child->children = nullptr;
			continue;
		}

		logger.debug("fillNode recursively '%s'", m->name);
		auto _ = logger.tabulator();

		std::shared_ptr<RealPrototype> rp = std::visit(
			[proto, child, &placer, &settings](const auto& memberProto) {
				using T = std::decay_t<decltype(memberProto)>;

				if constexpr (std::is_same_v<T, std::shared_ptr<RealPrototype>>) {
					return memberProto;

				} else {
					const VariadicPrototype* vp = memberProto.get();
					auto it = settings.find(vp->reference);
					if (it == settings.end()) {
						throw StructureError{
							"VariadicNotResolved",
							std::format(
								"Cannot resolve variadic '{}'",
								vp->reference->name
							)
						};
					}
					auto rp = *std::get_if<
						std::shared_ptr<RealPrototype>
					>(&it->second);

					
					if (rp) {
						logger.debug(
							"variadic '%s' resolved in '%s'",
							vp->reference->name,
							rp->clas.getSymbol()
						);
						return rp;
					}

					logger.debug("Did'nt found variadic '%s'", vp->reference->name);
					return std::shared_ptr<RealPrototype>{nullptr};
				}
			},
			m->proto
		);

		if (rp == nullptr) {
			child->placement.size = MEMORY_SIZE_UNKNOWN;
			child->placement.offset = -1;
			child->cl = nullptr;
			placer.add(MEMORY_SIZE_UNKNOWN);
			continue;
		}

		// Replace settings
		std::map<const ClassMember*, Prototype> subSettings;
		for (auto [member, p]: rp->settings) {
			std::visit([member, &settings, &subSettings](const auto& p) {
				using T = std::decay_t<decltype(p)>;

				if constexpr (std::is_same_v<T, std::shared_ptr<RealPrototype>>) {
					subSettings.insert({member, {p}});

				} else {
					auto it = settings.find(p->reference);
					if (it == settings.end()) {
						throw StructureError{
							"VariadicNotResolved",
							std::format("Cannot resolve variadic '{}'", p->reference->name)
						};
					}

					subSettings.insert({member, it->second});
				}
			}, p);
		}

		fillNode(child, subSettings, rp);
		child->placement.offset = placer.add(child->placement.size);
		logger.debug(
			"fillNode '%s' resolved at (%d, %d, +%d)",
			m->name,
			child->placement.size.size,
			child->placement.size.align,
			child->placement.offset
		);
	}

	node->placement.size = placer.evalSize();
	node->children = std::move(children);
	node->cl = proto->clas.force();
	logger.debug("define %p", node->cl);
}

void Variable::resolvePlacements() {
	if (this->position) {
		return; // already resolved
	}

	auto _ = logger.tabulator();

	std::visit([this](const auto& proto) {
		using T = std::decay_t<decltype(proto)>;

		if constexpr (std::is_same_v<T, std::shared_ptr<RealPrototype>>) {
			auto node = new PositionNode{};
			const std::map<const ClassMember*, Prototype> startSettings{};
			fillNode(node, startSettings, proto);
			this->position = node;
		} else {
			throw StructureError{
				"VariadicResolutionError", 
				"Could not resolve VariadicPrototype in a variable"
			};
		}
	}, this->proto);

}



static std::string printPlacementArray(label_t* array, size_t length) {
	std::string labels;
	for (size_t i = 0; i < length; ++i) {
		if (i > 0)
			labels += ", ";

		labels += array[i];
	}

	return labels;
}

MemoryPlacement Variable::getPlacement(label_t* array, size_t length) {
	logger.debug(
		"getPlacement inside '%s':'%s'",
		this->name,
		this->proto.getMinimalClass().getSymbol()
	);


	if (std::get_if<std::shared_ptr<VariadicPrototype>>(&this->proto)) {
		/// TODO: handle starting with VariadicPrototype
		return {
			.size = MEMORY_SIZE_UNKNOWN,
			.offset = -1
		};
	}

	auto rp = *std::get_if<std::shared_ptr<RealPrototype>>(&this->proto);

	auto cl = rp->clas.force();
	
	// Placements are all entire
	if (cl->isEntire()) {
		int offset = 0;
		const Class* current = cl;
		loopPtr(labelPtr, array, length) {
			auto m = current->members.search(*labelPtr);
			if (m == nullptr) {
				throw StructureError{
					"UnresolvedMemberError",
					std::format(
						"Cannot resolve member '{}' from '{}'",
						*labelPtr, current->name
					)
				};
			}

			offset += m->placement.offset;
			current = m->proto.getMinimalClass().force();
		}

		{
			logger.debug(std::format(
				"Entire placement of [{}] is {} {} {}",
				printPlacementArray(array, length),
				current->size.size,
				current->size.align,
				offset
			));

			return {
				.size = current->size,
				.offset = offset
			};
		}

		return {
			.size = current->size,
			.offset = offset
		};
	}


	// Enshure position is defined
	PositionNode* position;
	if (this->position == nullptr) {
		// Define position
		logger.debug("Define position of variable '%s'", this->name);
		
		auto _ = Logger::tabulator();
		position = new PositionNode;
		fillNode(position, rp->settings, rp);
		this->position = position;
	} else {
		position = this->position;
	}


	// Run accross positions
	int offset = 0;
	MemorySize size;
	loopPtr(labelPtr, array, length) {
		auto m = cl->members.search(*labelPtr);
		if (m == nullptr) {
			throw StructureError{
				"UnresolvedMemberError",
				std::format(
					"Cannot resolve member '{}' from '{}'",
					*labelPtr,
					cl->name
				)
			};
		}

		if (position) {
			// Find the member's position in the PositionNode children array.
			size_t index = 0;
			bool found = false;
			
			for (
				auto it = cl->members.trace_cbegin();
				it != cl->members.trace_cend();
				it++, index++
			) {
				if (*it == m) {
					found = true;
					break;
				}
			}

			if (!found) {
				throw StructureError{
					"CannotFindMemberError",
					std::format(
						"Member '{}' is not present in its class member list",
						m->name
					)
				};
			}

			if (position->children == nullptr) {
				goto applyEntire;
			}

			position = &position->children[index];
			logger.debug("Take ( graph) %s.%s (+%d)", cl->name, m->name, position->placement.offset);
			offset += position->placement.offset;

			if (labelPtr == end_labelPtr - 1) {
				size = position->placement.size;
			}
	
			cl = position->cl;

			if (cl == nullptr) {
				throw StructureError{
					"EmptyEntireClassError",
					std::format(
						"Uncountred empty class on member '%s'",
						m->name
					)
				};
			}
		
		} else if (cl->isEntire()) {
			applyEntire:
			logger.debug("Take (direct) %s.%s (+%d)", cl->name, m->name, m->placement.offset);
			offset += m->placement.offset;
			cl = m->proto.getMinimalClass().force();

			if (labelPtr == end_labelPtr - 1) {
				size = m->placement.size;
			}

		} else {
			throw StructureError{
				"NotEntireClassError",
				std::format(
					"Uncountred notEntire class"
				)
			};
		}
	}

	
	/// TODO: return
	return {size, offset};
	

}







Variable::~Variable() {
	delete this->position;
};
