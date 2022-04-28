#include <iostream>
#include "Solver.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <chrono>

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
	//auto CNF = readDimacs("C:/Users/aaron/Desktop/dimacs/jnh2_unsat.cnf");
	//auto CNF = readDimacs("C:/Users/aaron/Desktop/dimacs/jnh1_sat.cnf");

	std::vector<std::string> testFiles = { "C:/Users/aaron/Desktop/dimacs/jnh2_unsat.cnf",
										   "C:/Users/aaron/Desktop/dimacs/jnh1_sat.cnf",
										   "C:/Users/aaron/Desktop/dimacs/jnh3_unsat.cnf",
										   "C:/Users/aaron/Desktop/dimacs/jnh7_sat.cnf",
										   "C:/Users/aaron/Desktop/dimacs/jnh218_sat.cnf",
										   "C:/Users/aaron/Desktop/dimacs/jnh309_unsat.cnf",
										   "C:/Users/aaron/Desktop/dimacs/flat200-22_sat.cnf"
	};

	auto start = std::chrono::high_resolution_clock::now();


	// Perform unit tests on each file.
	for (auto file : testFiles) {

		std::cout << "Processing unit test " << file << " |";
		auto CNF = readDimacs(file);

		// target is "true" if "unsat" not in the filename. 
		bool target = file.find("unsat") == std::string::npos;

		int numRuns = 10000;
		int tenth = numRuns / 10;
		for (int i = 0; i < 10000; ++i) {

			if ( i && i % tenth == 0) std::cout << "X";

			//std::cout << "\nRun " << i << "\n";
			Solver S(CNF);
			auto solution = S.Solve();
			if (solution.front() != target) {
				std::cout << "Unit test failed on file " << file << "\n";
				std::cin.get();
			}
		}

		std::cout << "\n";
	}

	std::cout << "\n\nAll unit tests successfully completed" << "\n";
	auto finish = std::chrono::high_resolution_clock::now();
	auto duration = std::chrono::duration_cast<std::chrono::seconds>(finish - start);
	std::cout << "Elapsed time: " << duration.count() << "\n";
	std::cin.get();
	return 0;
}