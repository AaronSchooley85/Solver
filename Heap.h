
#ifndef HEAP_H
#define HEAP_H

#include <vector>
#include <stdlib.h>
#include <algorithm>
#include "Heap.h"
#include "Variable.h"

class Heap {

public:

	// Remove from, and add to heap, respectively.
	Variable* pop(bool random = false);
	void push(Variable*);
	void reheapify();
	void setSeed(int s);

private:
	int seed = 0;
	std::vector<Variable*> heap;
};

#endif