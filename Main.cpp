#include <iostream>
#include "Solver.h"

int main() {

	std::vector<std::vector<int>> CNF{ {1,3}, {2, -3, 5} };
	Solver S(CNF);
	std::cin.get();
	return 0;
}