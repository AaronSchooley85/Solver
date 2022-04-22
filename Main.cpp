#include <iostream>
#include "Solver.h"

int main() {

	std::vector<std::vector<int>> CNF{ {1}, {-2, -3, 5}, {2}, {-5, -1, 2} };
	Solver S(CNF);
	S.Solve();
	std::cin.get();
	return 0;
}