#include <iostream>
#include "Solver.h"

int main() {

	//std::vector<std::vector<int>> CNF{ {1,2}, {-1, 3}, {2, -3}, {-2, -4}, {-3, 4} }; // 1 -> false, 2 -> true, 3 -> false, 4 -> false
	//std::vector<std::vector<int>> CNF{ {1,2,-3} , {2,3,-4} , {3,4,1} , {4,-1,2} , {-1,-2,3} , {-2,-3,4} , {-3,-4,-1} }; // Solution 1 -> false, 2 -> true, 4 -> true
	std::vector<std::vector<int>> CNF{ {1,2,-3} , {2,3,-4} , {3,4,1} , {4,-1,2} , {-1,-2,3} , {-2,-3,4} , {-3,-4,-1} , {-4,1,-2} }; // Unsat
	for (int i = 0; i < 1000; ++i) {

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
	}
	std::cout << "Complete" << "\n";
	std::cin.get();
	return 0;
}