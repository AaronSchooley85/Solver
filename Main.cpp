#include <iostream>
#include "Solver.h"

int main() {

	std::vector<std::vector<int>> CNF{ {1,2}, {-1, 3}, {2, -3}, {-2, -4}, {-3, 4} };
	Solver S(CNF);
	S.Solve();
	std::cin.get();
	return 0;
}