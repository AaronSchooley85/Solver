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

	// Loop through the CNF and process each clause within.
	for (auto& clause : CNF) {

		// This sat solver encodes literals such that a positive literal l becomes 2*l
		// and a negative literal l becomes 2*l + 1
		std::vector<int> encoded(clause);
		std::transform(clause.begin(), clause.end(), encoded.begin(), [](int a) {return 2 * std::abs(a) + (a < 0); });

		// Ensure that a variable object exists corresponding to each literal in the clause.
		for (auto& literal : encoded) {
			size_t variableNumber = literal >> 1;
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
				clauses.back().setClauseNumber(clauseNumber);
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

	// Record the number of variables in the problem.
	n = variables.size() - 1;

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
		// F == G ? 
		if (G == trail.size()) {

			// If we are finished.
			if (trail.size() == n) {
				
				// TODO - construct and return boolean vector.
				std::cout << "Congratulations\n";
				std::cin.get();
			}
			// Not finished. We need to make a decision.
			else {

				// Select a free variable from the heap and place on trail.
				// Will result in F = G + 1.
				makeADecision();
			}

		}

		// Process next literal on trail pointed at by G and increment G.
		auto literal = trail.at(G++);

		auto conflictEncountered = checkForcing(literal);
	}

}

// Knuth step C4.
// Check if the literal being processed forces other literals to take values.
// Return value is status of conflict. True -> conflict detected. False -> no conflict.
bool Solver::checkForcing(int literal) {

	// Get the variable object corresponding to the literals.
	auto& variable = vfl(literal);

	// Algorithm C requires the literal at index 1 of the clause to be the contradicted literal.
	auto contradictedLiteral = literal ^ 1;

	// Process each clause which watches the contradicted polarity of the literal.
	auto& contradictedWatchers = variable.getContradictedWatchers();
	std::vector<int> contradictedWatcherIndicesToProcess(contradictedWatchers.begin(), contradictedWatchers.end());

	// Inspect each contradicted clause. Below we remove any if necessary from variable's collection.
	for (auto contradictedClauseNumber : contradictedWatcherIndicesToProcess){

		// Get the clause which contains the contradicted literal and its literals.
		auto& contradictedClause = clauses.at(contradictedClauseNumber);
		auto& contradictedClauseLiterals = contradictedClause.getLiterals();

		// Swap first two literals if the element at index 1 is not the contradicted literal.
		if (contradictedClauseLiterals.at(1) != contradictedLiteral) {
			std::iter_swap(contradictedClauseLiterals.begin(), contradictedClauseLiterals.begin() + 1);
		}

		// If literal at index 0 is not true (i.e. false or unset).
		auto l0 = contradictedClauseLiterals.front();
		auto& v0 = vfl(l0);
		if (!v0.isTrue(l0)) {
			
			bool swapSuccess = false;
			for (size_t i = 2, len = contradictedClauseLiterals.size(); i < len; ++i) {

				// Get the candidate literal and its associated variable object.
				auto lx = contradictedClauseLiterals.at(i);
				auto& vx = vfl(lx);

				// If the new variable has not been set false.
				if (!vx.isFalse(lx)) {

					// Swap elements, add clause to new watched variable, and remove from old variable's watch.
					std::swap(contradictedClauseLiterals.at(0), contradictedClauseLiterals.at(i));
					vx.addToWatch(contradictedClause.getClauseNumber(), !(lx & 1));

					// Remove clause number from watch list of previously watched variable. Exchange and pop off back.
				    auto indexToRemove = std::find(contradictedWatchers.begin(), contradictedWatchers.end(), contradictedClauseNumber);
					std::iter_swap(indexToRemove, contradictedWatchers.end() -1);
					contradictedWatchers.pop_back();
					swapSuccess = true;
					break;
				}
			}

			// If we could not swap with another variable, we must check literal at clause index 0.
			if (!swapSuccess) {

				// If v0 is free, set it such that it is true. Note that we could not change the
				// watched literal which was in contradiction. But because l0 is now true, this
				// step C4 will skip this clause and not care (see above logic).
				if (v0.isFree()) {
					addForcedLiteralToTrail(l0, contradictedClauseNumber);
				}
				// We must resolve a conflict.
				else {
					std::cout << "Resolve\n";
				}
			}
		}
	}

	return false;

}

// Knuth step C6.
void Solver::makeADecision() {

	// Record trail index at which this level begins.
	levels.push_back(trail.size());
	Variable* nextFree = nullptr;
	do {
		nextFree = heap.pop();
	} while (!nextFree->isFree());
	
	// Add the new decision variable to the trail.
	// This will cause F = G + 1
	addDecisionVariableToTrail(nextFree->getVariableNumber());
}

// Construct a new clause.
void Solver::resolveConflict(const std::vector<int>& clause) {
	
	// If there have been no decision levels created, we failed.
	if (!depth()) {
		solutionFailed = true;
		return;
	}

	// Data needed for blit routine below.
	int count = 0;
	std::vector<int> b{ -1 }; // 'r' is the size of b. Placeholder at index 0.
	size_t stamp = nextStamp();
	int dprime = 0;

	// Process the first literal in the clause.
	auto l0 = clause.front();
	auto& v0 = vfl(l0);
	v0.setStamp(stamp); // Stamped here, not in 'blit' so count is not affected. 
	v0.bumpActivity(DEL);

	// Local function to process 'b' literals.
	auto blit = [&](int literal) {

		auto& v = vfl(literal);
		if (v.getStamp() != stamp) {
			v.setStamp(stamp);
			auto p = v.getValue() >> 1;
			if (p > 0) {
				v.bumpActivity(DEL);
				if (p == depth()) {
					++count;
				}
				else {
					b.push_back(literal ^ 1);
					dprime = std::max(p, dprime);
				}
			}
		}
	};

	// Apply blit algorithm to all literals at index GREATER THAN 0 in clause.
	for (size_t i = 1, len = clause.size(); i < len; ++i) blit(clause.at(i));

	// Get the highest trail index of ALL literals in the clause.
	int t = 0;
	for (auto lit : clause) t = std::max(vfl(lit).getTloc(), t);

	while (count > 0) {
		auto l = trail.at(t--); // Get literal furthest up the trail.
		auto& v = vfl(l);
		count -= (v.getStamp() == stamp); // Branchless, conditional decrement if stamp matches.
		int reasonIndex = v.getReason();

		// Process a reason if it exists. 
		if (reasonIndex != -1) {
			auto& reasonClause = clauses.at(reasonIndex).getLiterals();
			for (size_t i = 1, len = reasonClause.size(); i < len; ++i) blit(reasonClause.at(i));
		}
	}

	// Find the last stamped literal.
	auto lprime = trail.at(t--);
	while (vfl(lprime).getStamp() != stamp) lprime = trail.at(t--);
	b.front() = lprime ^ 1; // Replace placeholder.
}

void Solver::Learn(std::vector<int>& clause, int dprime) {

}
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

// Convenience methods to get variable objects.
Variable& Solver::vfl(int literal) { return variables.at(literal >> 1); }
Variable& Solver::vfv(int variableNumber) { return variables.at(variableNumber); }

size_t Solver::nextStamp() {
	stamp += 3;
	return stamp;
}

// Index of levels is the same as level. So level 1 begins
// at index 1. Level 0 at index zero is always equal to zero
// and doesn't count toward the level count. Therefore with only
// zero in index zero, we have a depth of zero (size - 1).
int Solver::depth() { return levels.size(); }
