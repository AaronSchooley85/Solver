#include "Heap.h"
#include <iostream>

struct comparison {

	bool operator()(const Variable* a, const Variable* b) const {
		return a->getActivity() < b->getActivity();
		}
};

void Heap::push(Variable* v) {

	heap.push_back(v);
	std::push_heap(heap.begin(), heap.end(), comparison());
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

