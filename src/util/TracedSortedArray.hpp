#include <algorithm>
#include <cstddef>
#include <memory>
#include <vector>

#include "SortedArray.hpp"

namespace tracedSortedArrayDetail {

	// Comparator adapter used by SortedArray<T*, ...>.
	// It compares the pointed objects instead of the pointers themselves.
	template<typename T, typename Comparator>
	struct PointerComparator {
		auto operator()(T* lhs, T* rhs) const
			-> decltype(Comparator{}(*lhs, *rhs)) {
			return Comparator{}(*lhs, *rhs);
		}

		template<typename Key>
		auto operator()(T* lhs, const Key& rhs) const
			-> decltype(Comparator{}(*lhs, rhs)) {
			return Comparator{}(*lhs, rhs);
		}
	};

}

/*
 * Non-owning sorted array of T*.
 *
 * The array stores the pointers themselves and never takes ownership
 * of the pointed objects.
 *
 * The main array is sorted according to Comparator applied to T.
 * The trace array keeps the original order of the pointers.
 */
template<typename T, typename Key, typename Comparator>
class TracedSortedArray
	: public SortedArray<
		T*,
		Key,
		tracedSortedArrayDetail::PointerComparator<T, Comparator>
	> {

private:
	// Pointers in their original order.
	std::unique_ptr<T*[]> trace;

public:
	using Base = SortedArray<
		T*,
		Key,
		tracedSortedArrayDetail::PointerComparator<T, Comparator>
	>;

	// Iterators over the trace array.
	using trace_iterator = T**;
	using const_trace_iterator = T* const*;

	// Default constructor.
	TracedSortedArray()
		: Base(), trace(nullptr) {}

	// Build the array from a vector of non-owning pointers.
	TracedSortedArray(const std::vector<T*>& elements)
		: Base(), trace(nullptr) {

		this->length = elements.size();

		if (this->length == 0)
			return;

		this->start = std::make_unique<T*[]>(this->length);
		trace = std::make_unique<T*[]>(this->length);

		struct Entry {
			T* ptr;
			std::size_t originalIndex;
		};

		auto entries = std::make_unique<Entry[]>(this->length);

		// Store the pointers together with their original indices.
		for (std::size_t i = 0; i < this->length; ++i) {
			entries[i] = {
				elements[i],
				i
			};
		}

		// Sort according to the pointed objects.
		std::sort(
			entries.get(),
			entries.get() + this->length,
			[](const Entry& lhs, const Entry& rhs) {
				return Comparator{}(*lhs.ptr, *rhs.ptr) < 0;
			}
		);

		// Store the sorted pointers in the main array.
		for (std::size_t sortedIndex = 0;
			 sortedIndex < this->length;
			 ++sortedIndex) {

			this->start[sortedIndex] = entries[sortedIndex].ptr;
		}

		// Rebuild the original ordering.
		for (std::size_t sortedIndex = 0;
			 sortedIndex < this->length;
			 ++sortedIndex) {

			const std::size_t originalIndex =
				entries[sortedIndex].originalIndex;

			trace[originalIndex] = entries[sortedIndex].ptr;
		}
	}

	// Copy constructor.
	TracedSortedArray(const TracedSortedArray& ref)
		: Base(ref), trace(nullptr) {

		if (this->length == 0)
			return;

		trace = std::make_unique<T*[]>(this->length);

		// The pointers themselves are copied.
		// The pointed objects are not copied or owned.
		for (std::size_t i = 0; i < this->length; ++i)
			trace[i] = ref.trace[i];
	}

	// Move constructor.
	TracedSortedArray(TracedSortedArray&&) noexcept = default;

	// Copy assignment operator.
	TracedSortedArray& operator=(const TracedSortedArray& ref) {
		if (this == &ref)
			return *this;

		Base::operator=(ref);

		if (this->length == 0) {
			trace.reset();
			return *this;
		}

		trace = std::make_unique<T*[]>(this->length);

		for (std::size_t i = 0; i < this->length; ++i)
			trace[i] = ref.trace[i];

		return *this;
	}

	// Move assignment operator.
	TracedSortedArray& operator=(TracedSortedArray&&) noexcept = default;

	/*
	 * Access an element of the sorted array.
	 *
	 * The pointed object is not returned by reference:
	 * the returned value is the stored T*.
	 */
	T* operator[](std::size_t index) {
		return this->start[index];
	}

	const T* operator[](std::size_t index) const {
		return this->start[index];
	}

	// Trace iterators.
	trace_iterator trace_begin() {
		return trace.get();
	}

	trace_iterator trace_end() {
		return trace.get() + this->length;
	}

	const_trace_iterator trace_begin() const {
		return trace.get();
	}

	const_trace_iterator trace_end() const {
		return trace.get() + this->length;
	}

	const_trace_iterator trace_cbegin() const {
		return trace.get();
	}

	const_trace_iterator trace_cend() const {
		return trace.get() + this->length;
	}

	T* search(const Key& key) const {
		auto ptr = Base::search(key);

		if (ptr == nullptr)
			return nullptr;

		return *ptr;
	}
};