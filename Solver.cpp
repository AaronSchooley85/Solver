#include "Solver.h"
#include <iostream>

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
		// Unit clauses go directly to trail instead.
		if (!clause.size()) {
			std::cout << "Empty clause found in solver initialization. No solution possible.";
			std::cin.get();
			exit(1);
		}
		else if (clause.size() == 1) {
			auto literal = clause.front();
			auto& variable = vfl(literal);

			// Mismatch if not free and polarity does not match.
			bool mismatch = variable.isFree() ? false : (variable.getValue() + literal) & 1;

			// Place literal on trail if it is not a mismatch.
			if (mismatch) {
				std::cout << "Mismatch between unit clauses. No solution found in initialization.";
				std::cin.get();
				exit(1);
			}
			else {
				addLiteralToTrail(clause.front());
			}
		}
		else {
			clauses.push_back(Clause(clause));
		}
	}
}

// Convenience methods to get variable objects.
Variable& Solver::vfl(int literal) { return variables.at(literal >> 1); }
Variable& Solver::vfv(int variableNumber) { return variables.at(variableNumber); }

void Solver::addLiteralToTrail(int literal) {

	auto& variable = vfl(literal);
	if (variable.isFree()) {
		variable.setValue(static_cast<int>(levels.size()), literal);
		variable.setTloc(static_cast<int>(trail.size()));
		trail.push_back(literal);
	}
}

// Add variable to trail. Polarity is determined by internal parity.
void Solver::addVariableToTrail(int variableNumber) {

}

// Add variable to trail with the specified polarity.
void Solver::addVariableToTrail(int variableNumber, bool b) {

}
