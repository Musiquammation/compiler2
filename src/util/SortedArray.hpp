#include <algorithm>
#include <cstddef>
#include <memory>

#include "../label_t.hpp"

template<typename T, typename Key, typename Comparator>
class SortedArray {
	std::unique_ptr<T[]> start;
	size_t length;

public:
	using iterator = T*;
	using const_iterator = const T*;

	SortedArray()
		: start(nullptr), length(0) {}

	SortedArray(std::unique_ptr<T[]>& ptr, size_t length)
		: start(std::move(ptr)), length(length) {}

	SortedArray(const SortedArray& ref)
		: start(std::make_unique<T[]>(ref.length)),
		  length(ref.length) {
		std::copy(
			ref.start.get(),
			ref.start.get() + length,
			start.get()
		);
	}

	SortedArray(SortedArray&&) noexcept = default;

	SortedArray& operator=(const SortedArray& ref) {
		if (this == &ref)
			return *this;

		start = std::make_unique<T[]>(ref.length);
		length = ref.length;

		std::copy(
			ref.start.get(),
			ref.start.get() + length,
			start.get()
		);

		return *this;
	}

	SortedArray& operator=(SortedArray&&) noexcept = default;

	iterator begin() {
		return start.get();
	}

	iterator end() {
		return start.get() + length;
	}

	const_iterator begin() const {
		return start.get();
	}

	const_iterator end() const {
		return start.get() + length;
	}

	const_iterator cbegin() const {
		return start.get();
	}

	const_iterator cend() const {
		return start.get() + length;
	}

	T* search(const Key& key) const {
		size_t left = 0;
		size_t right = length;

		while (left < right) {
			size_t mid = left + (right - left) / 2;

			int cmp = Comparator{}(start[mid], key);

			if (cmp < 0) {
				left = mid + 1;
			} else if (cmp > 0) {
				right = mid;
			} else {
				return &start[mid];
			}
		}

		return nullptr;
	}
};

template<typename T>
struct LabeledComparator {
	int operator()(const T& obj, label_t label) const {
		return obj.name - label;
	}
};

