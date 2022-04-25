#include <iostream>
#include "Solver.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

std::vector<std::vector<int>> readDimacs(std::string filepath) {

	std::vector<std::vector<int>> CNF;
	std::ifstream dimacsFile(filepath);

	std::string text;

	while (std::getline(dimacsFile, text)) {
		std::istringstream iss(text);
		std::vector<int> clause{ std::istream_iterator<int>{iss}, std::istream_iterator<int>{} };
		if (clause.back() == 0) clause.pop_back(); // Remove trailing zero if it exists. 
		CNF.push_back(clause);
	}

	return CNF;
}

int main() {

	//std::vector<std::vector<int>> CNF{ {1,2}, {-1, 3}, {2, -3}, {-2, -4}, {-3, 4} }; // 1 -> false, 2 -> true, 3 -> false, 4 -> false
	//std::vector<std::vector<int>> CNF{ {1,2,-3} , {2,3,-4} , {3,4,1} , {4,-1,2} , {-1,-2,3} , {-2,-3,4} , {-3,-4,-1} }; // Solution 1 -> false, 2 -> true, 4 -> true
	//std::vector<std::vector<int>> CNF{ {1,2,-3} , {2,3,-4} , {3,4,1} , {4,-1,2} , {-1,-2,3} , {-2,-3,4} , {-3,-4,-1} , {-4,1,-2} }; // Unsat
	auto CNF = readDimacs("C:/Users/aaron/Desktop/dimacs/jnh1_sat.cnf");
	for (int i = 0; i < 1000; ++i) {

		std::cout << "\nRun " << i << "\n";
		Solver S(CNF, i);
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