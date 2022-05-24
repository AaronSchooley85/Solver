#include <iostream>
#include "Solver.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <chrono>


std::vector<std::vector<int>> waerden(int j, int k, int n) {

	std::vector<std::vector<int>> cnf;


	int d = 1;
	bool run = false;
	do {
		run = false;
		for (int i = 1; i <= n - (j - 1) * d; ++i) {
			run = true;
			std::vector<int> tmp;
			for (int jj = 0; jj < j; ++jj) {
				tmp.push_back(i + (jj * d));
			}
			cnf.push_back(tmp);
		}
		++d;
	} while (run);


	d = 1;
	do {
		run = false;
		for (int i = 1; i <= n - (k - 1) * d; ++i) {
			run = true;
			std::vector<int> tmp;
			for (int kk = 0; kk < k; ++kk) {
				tmp.push_back(-(i + (kk * d)));
			}
			cnf.push_back(tmp);
		}
		++d;
	} while (run);

	return cnf;
}

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
										   "C:/Users/aaron/Desktop/dimacs/flat200-22_sat.cnf",
										   "C:/Users/aaron/Desktop/dimacs/hole6_unsat.cnf",
										   "C:/Users/aaron/Desktop/dimacs/hole9_unsat.cnf"
	};

	auto start = std::chrono::high_resolution_clock::now();

	std::cout << "Performing unit tests...\n\n";

	// Perform unit tests on each file.
	for (auto file : testFiles) {

		auto CNF = readDimacs(file);
		//auto CNF = waerden(5,5,178);
		//std::vector<std::vector<int>> CNF{ {-1,2,3} , {1,3} , {1,4} , {-4,6} };
		//std::vector<std::vector<int>> CNF{ {-1}, {-2} , {1,2}};

		// target is "true" if "unsat" not in the filename. 
		std::cout << "\r" << file << ": 0%";
		bool target = file.find("unsat") == std::string::npos;

		int numRuns = 1000;
		int hundredth = numRuns / 100;
		for (int i = 1; i <= numRuns; ++i) {

			if (hundredth ? i % hundredth == 0 : true) std::cout << "\r" << file << ": " << (100.0 * i) / numRuns << "%";

		//	std::cout << "\nRun " << i << "\n";
			Solver S(CNF, i);
			auto solution = S.Solve();
			if (solution.front() != target) {
				std::cout << "Unit test failed on file " << file << "\n";
				std::wcout << "Seed was: " << i << "\n";
				std::cin.get();
			}
		}

		std::cout << "\n";
	}

	std::cout << "\n\nAll unit tests successfully completed" << "\n";
	auto finish = std::chrono::high_resolution_clock::now();
	auto duration = std::chrono::duration_cast<std::chrono::seconds>(finish - start);
	std::cout << "Elapsed time: " << duration.count() << " seconds\n";
	std::cin.get();
	return 0;
}