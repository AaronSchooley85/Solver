#include "Solver.h"
#include <iostream>

#define DEBUG

// Solver constructor. Initializes variables, loads clauses, and processes
// unit clauses. 
Solver::Solver(cnf CNF) {

	// We will use 1-based indexing to align with Knuth's text.
	// Create dummy entries here.
	variables.push_back(Variable(0));
	clauses.push_back(Clause({}));
	levels.push_back(0); // Decision zero is always at index 0.

	// Loop through the CNF and process each clause within.
	for (auto& clause : CNF) {

		// This sat solver encodes literals such that a positive literal l becomes 2*l
		// and a negative literal l becomes 2*l + 1
		std::vector<int> encoded(clause);
		std::transform(clause.begin(), clause.end(), encoded.begin(), [](int a) {return 2 * std::abs(a) + (a < 0); });

		// Ensure that a variable object exists corresponding to each literal in the clause.
		for (auto& literal : encoded) {
			auto variableNumber = literal >> 1;
			while (variables.size() <= variableNumber) variables.push_back(Variable(variables.size()));
		}

		// Add the clause to our collection of clauses. Stored encoded via "2k/2k+1" scheme.
		switch (encoded.size()) {

			// Empty clauses cause immediate failure.
			case 0:
				std::cout << "Empty clause found in solver initialization. No solution possible.";
				std::cin.get();
				exit(1);
				break;

			// Unit clauses go strait to trail if no contradiction found, otherwise fail.
			// Watch lists are not set for unit clauses. TODO - Verify.
			case 1: {
				auto literal = encoded.front();
				auto& variable = vfl(literal);

				// Mismatch if not free and polarity does not match.
				bool contradiction = variable.isFree() ? false : (variable.getValue() + literal) & 1;

				// Place literal on trail if it is not a mismatch.
				if (contradiction) {
					std::cout << "Contradiction between unit clauses. No solution found in initialization.";
					std::cin.get();
					exit(1);
				}
				else {
					// Add the literal to trail. No reason for unit clauses.
					addForcedLiteralToTrail(encoded.front(), -1);
				}
				break; }

			// Standard clause of length greater than 1. 
			default:
				auto clauseNumber = clauses.size();
				clauses.push_back(Clause(encoded));
				auto& l0 = clauses.back().getLiterals().at(0);
				auto& l1 = clauses.back().getLiterals().at(1);
				auto& v0 = vfl(l0);
				auto& v1 = vfl(l1);
				v0.addToWatch(clauseNumber, l0 % 2 == 0);
				v1.addToWatch(clauseNumber, l1 % 2 == 0);
		}
	}

	// Record the index for the end of the clause vector.
	minl = static_cast<int>(clauses.size());

	// Add free variables to heap.
	for (int i = 1; i < variables.size(); ++i) {
		auto& variable = variables.at(i);
		if (variable.isFree()) heap.push(&variable);
	}
}

// Entry point to begin solving the CNF supplied to the constructor.
std::vector<bool> Solver::Solve() {

	while (true){

		// If we are out of variables to process we either finished
		// the problem or need to make a decision on the next variable. 
		if (G == trail.size()) {

			// If we are finished.
			if (trail.size() == variables.size()) {
				
				// TODO - construct and return boolean vector.
			}
			// Not finished. We need to make a decision.
			else {

				// Record trail index at which this level begins.
				levels.push_back(trail.size());
				Variable* nextFree = nullptr;
				do {
					nextFree = heap.pop();
				} while (!nextFree->isFree());
				
				// Set free variale to value determined by old value.

			}

		}
	}

}

// Convenience methods to get variable objects.
Variable& Solver::vfl(int literal) { return variables.at(literal >> 1); }
Variable& Solver::vfv(int variableNumber) { return variables.at(variableNumber); }

// Add a variable to trail. The value is determined by the oval property. There is
// no reason since it was a decision.
void Solver::addDecisionVariableToTrail(int variableNumber) {

	auto& variable = vfv(variableNumber);
	if (variable.isFree()) {
		variable.setValue(static_cast<int>(levels.size()));
		variable.setTloc(static_cast<int>(trail.size()));
		variable.setReason(-1);
		trail.push_back(variable.getCurrentLiteralValue());
	}
#ifdef DEBUG
	else {
		std::cout << "Placing a variable which is not free on trail!\n";
		std::cin.get();
		exit(1);
	}
#endif
}

// Place a literal on the trail which was forced by some clause.
// The value is determined by the level and the literal value.
void Solver::addForcedLiteralToTrail(int literal, int reason) {

	auto& variable = vfl(literal);
	if (variable.isFree()) {
		variable.setValue(static_cast<int>(levels.size()), literal);
		variable.setTloc(static_cast<int>(trail.size()));
		variable.setReason(reason);
		trail.push_back(variable.getCurrentLiteralValue());
	
#ifdef DEBUG 
		if (literal != variable.getCurrentLiteralValue()) {
			std::cout << "Looks like a bug!\n";
			std::cin.get();
			exit(1);
		}

	}
	else {
		std::cout << "Placing a variable which is not free on trail!\n";
		std::cin.get();
		exit(1);
	}
#endif
}


// Add variable to trail. Polarity is determined by internal parity.
void Solver::addVariableToTrail(int variableNumber) {

}

// Add variable to trail with the specified polarity.
void Solver::addVariableToTrail(int variableNumber, bool b) {

}
