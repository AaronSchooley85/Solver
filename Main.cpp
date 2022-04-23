#include <iostream>
#include "Solver.h"

int main() {

	std::vector<std::vector<int>> CNF{ {1,2}, {-1, 3}, {2, -3}, {-2, -4}, {-3, 4} };
	Solver S(CNF);
	auto solution = S.Solve();
	if (solution.front()) {
		std::cout << "SATISFIABLE\n";
		for (auto it = solution.begin() + 1; it != solution.end(); ++it) {
			std::cout << it - solution.begin() << ((*it) ? " true" : " false");
			std::cout << "\n";
		}
	}
	else {
		std::cout << "UNSATISFIABLE\n";
	}
	std::cin.get();
	return 0;
}