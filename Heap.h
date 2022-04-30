
#ifndef HEAP_H
#define HEAP_H

#include <vector>
#include <algorithm>
#include "Heap.h"
#include "Variable.h"

class Heap {

public:

	// Remove from, and add to heap, respectively.
	Variable* pop();
	void push(Variable*);
	void reheapify();

private:
	std::vector<Variable*> heap;
};

#endif