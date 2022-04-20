#include "Heap.h"

void Heap::push(Variable* v) {

	heap.push_back(v);
	std::push_heap(heap.begin(), heap.end());
	v->setHloc(true);
}

// Return the largest element.
Variable* Heap::pop() {
	std::pop_heap(heap.begin(), heap.end());
	auto max = heap.back();
	heap.pop_back();
	max->setHloc(false);
	return max;
}

bool comparison (const Variable& a, const Variable& b) {
	return a.getActivity() < b.getActivity();
	}
