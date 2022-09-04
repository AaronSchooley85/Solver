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
Variable* Heap::pop(bool random) {

	// Occasionally place a random item on the back.
	if (random && (rand() % 1000) < 20) {

		int randomIndex = rand() % heap.size();
		std::iter_swap(heap.begin() + randomIndex, heap.end() - 1);
	}
	// Usually put the highest activity variable on the back.
	else {
		std::pop_heap(heap.begin(), heap.end(), comparison());
	}

	Variable* max = heap.back();
	heap.pop_back();
	max->setHloc(false);
	return max;
}

// Return the variable with the highest activity score which is
// free. Nothing is removed from the heap. Heap is not modified.
Variable* Heap::queryMaxFreeVariable() {
	
	Variable* v;
	int i = 0;
	do {
		v = heap[i++];
	} while (!v->isFree());

	return v;
}

// Intended use is to restore heap property when activity scores have
// been externally modified by solver. 
void Heap::reheapify() {

	std::make_heap(heap.begin(), heap.end(), comparison());
}

void Heap::setSeed(int s) { 
	seed = s;
	srand(seed);
}

