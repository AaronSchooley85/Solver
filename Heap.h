
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

private:
	std::vector<Variable*> heap;
};

#endif