#include "Solver.h"

Solver::Solver(cnf CNF) {

	// We will use 1-based indexing to align with Knuth's text.
	// Create dummy entries here.
	variables.push_back(Variable(0));
	clauses.push_back(Clause({}));

	// Loop through the CNF. Add the clauses and create the variables.
	for (auto& clause : CNF) {

		// For each literal in the clause, ensure that a variable object exists.
		for (auto& literal : clause) {
			auto variableNumber = std::abs(literal);
			while (variables.size() <= variableNumber) variables.push_back(Variable(variables.size()));
		}

		// Add the clause to our collection of clauses. Stored encoded via "2k/2k+1" scheme.
		clauses.push_back(Clause(clause));
	}
}