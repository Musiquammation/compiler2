#include <algorithm>
#include <cstddef>
#include <memory>
#include <utility>
#include "SortedArray.hpp"

template<typename T, typename Key, typename Comparator>
class TracedSortedArray : public SortedArray<T, Key, Comparator> {
private:
	// Secondary array storing raw pointers in their initial (unsorted) order
	std::unique_ptr<T*[]> trace;

public:
	// Iterator aliases for the trace array
	using trace_iterator = T**;
	using const_trace_iterator = T* const*;

	// Default constructor
	TracedSortedArray()
		: SortedArray<T, Key, Comparator>(), trace(nullptr) {}

	// Constructor taking ownership of the input array and recording initial pointers
	TracedSortedArray(std::unique_ptr<T[]>& ptr, size_t length): 
	SortedArray<T, Key, Comparator>() {
		this->length = length;
		this->start = std::move(ptr);

		if (length == 0)
			return;

		struct Entry {
			T* ptr;
			size_t originalIndex;
		};

		auto entries = std::make_unique<Entry[]>(length);

		// Keep pointers to the objects in their original order.
		for (size_t i = 0; i < length; ++i) {
			entries[i] = {
				&this->start[i],
				i
			};
		}

		// Sort the pointers according to the pointed values.
		std::sort(
			entries.get(),
			entries.get() + length,
			[](const Entry& a, const Entry& b) {
				return Comparator{}(*a.ptr, *b.ptr) < 0;
			}
		);

		// Store the sorted values in a temporary array.
		auto sorted = std::make_unique<T[]>(length);

		// Buffer
		for (size_t i = 0; i < length; ++i)
			sorted[i] = std::move(*entries[i].ptr);

		// Move the sorted values back into start.
		for (size_t i = 0; i < length; ++i)
			this->start[i] = std::move(sorted[i]);

		// trace uses the original indices.
		trace = std::make_unique<T*[]>(length);

		for (size_t sortedIndex = 0; sortedIndex < length; ++sortedIndex) {
			trace[entries[sortedIndex].originalIndex] = &this->start[sortedIndex];
		}
	}

	// Copy constructor
	TracedSortedArray(const TracedSortedArray<T, Key, Comparator>& ref)
		: SortedArray<T, Key, Comparator>(ref) {
		rebuild_trace_from(ref);
	}

	// Move constructor
	TracedSortedArray(TracedSortedArray&&) noexcept = default;

	// Copy assignment operator
	TracedSortedArray& operator=(const TracedSortedArray& ref) {
		if (this == &ref)
			return *this;

		SortedArray<T, Key, Comparator>::operator=(ref);
		rebuild_trace_from(ref);

		return *this;
	}

	// Move assignment operator
	TracedSortedArray& operator=(TracedSortedArray&&) noexcept = default;

	// Trace iterators
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

private:
	// Helper function to re-map raw pointers after deep copying the base array
	void rebuild_trace_from(const TracedSortedArray& ref) {
		if (this->length == 0) {
			trace = nullptr;
			return;
		}

		trace = std::make_unique<T*[]>(this->length);
		for (size_t i = 0; i < this->length; ++i) {
			// Calculate relative offset in the original array and map to the new array
			std::ptrdiff_t offset = ref.trace[i] - ref.start.get();
			trace[i] = this->start.get() + offset;
		}
	}
};
