#include "Solver.h"
#include <iostream>
#include <random>
#include <chrono>


#define DEBUG

// Solver constructor. Initializes variables, loads clauses, and processes
// unit clauses. 
Solver::Solver(cnf CNF) {

	// We will use 1-based indexing to align with Knuth's text.
	// Create dummy entries here.
	variables.push_back(Variable(0));
	clauses.push_back(Clause({}));
	levels.push_back(0); // Level 0 (level k is at levels[k] ) always starts at trail index 0.

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
	std::cout << "Number of variables: " << n << "\n";
	std::cout << "Number of clauses: " << clauses.size() - 1 + trail.size() << "\n";
	std::cout << "Number of unit clauses: " << trail.size() << "\n";

	// Add free variables to heap.
	std::vector<Variable*> shuffledVariablePointers;
	for (int i = 1; i < variables.size(); ++i) shuffledVariablePointers.push_back(&variables.at(i));
	auto seed = std::chrono::system_clock::now().time_since_epoch().count();
	//seed = 7;
	std::shuffle(shuffledVariablePointers.begin(), shuffledVariablePointers.end(), std::default_random_engine(seed));
	for (auto &v : shuffledVariablePointers) {
		if (v->isFree()) heap.push(v);
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
				std::vector<bool> solution(n + 1);
				solution.front() = true;
				for (auto t : trail) solution.at(t >> 1) = t & 1 ? false : true;
				return solution;
			}
			// Not finished. We need to make a decision.
			else {

				// Select a free variable from the heap and place on trail.
				// Will result in F = G + 1.
				makeADecision();
			}
		}

		// Process next literal on trail pointed at by G and increment G.
		bool conflictEncountered = false;
		do {
			auto literal = trail.at(G++);
			conflictEncountered = checkForcing(literal);
			if (conflictEncountered && solutionFailed) return std::vector<bool>{false};
		} while (conflictEncountered);
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
					std::swap(contradictedClauseLiterals.at(1), contradictedClauseLiterals.at(i));
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
					resolveConflict(contradictedClauseLiterals);
					return true;
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
	if (depth() == 0) {
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
	bool rescale = false;
	rescale |= v0.bumpActivity(DEL);
	int currentDepth = depth();

	// Local function to process 'b' literals.
	auto blit = [&](int literal) {

		auto& v = vfl(literal);
		if (v.getStamp() != stamp) {
			v.setStamp(stamp);
			auto p = (v.getValue() >> 1);
			if (p > 0) {
				rescale |= v.bumpActivity(DEL); // Anytime we bump activity we may corrupt heap. Reheapify before popping heap? Set corrupted flag?
				count += (p == currentDepth);
				if (p < currentDepth) {
					b.push_back(literal);
					dprime = std::max(p, dprime);
				}
			}
		}
	};

#ifdef DEBUG
	if (dprime >= currentDepth) {
		std::cout << "d' is supposed to be less than depth\n";
		std::cin.get();
	}
#endif
	// Apply blit algorithm to all literals at index GREATER THAN 0 in clause.
	for (size_t i = 1, len = clause.size(); i < len; ++i) blit(clause.at(i));

	// If any variable had its activity score exceed our given threshold, rescale
	// all variables by that threshold. 
	if (rescale) {
		for (size_t i = 1, len = variables.size(); i < len; ++i) {
			auto &variable = variables.at(i);
			auto currentActivity = variable.getActivity();
			variable.setActivity(currentActivity / variable.threshold);
		}

		// Rescale DEL too.
		DEL /= variables.front().threshold;
	}

	// Get the highest trail index of ALL literals in the clause.
	int t = 0;
	for (auto lit : clause) {
		auto& v = vfl(lit);
		t = std::max(v.getTloc(), t); // Am I using 'max' correctly?
	}

	while (count > 0) {
		auto l = trail.at(t--); // Get literal furthest up the trail.
		auto& v = vfl(l);
		if (v.getStamp() == stamp) {

			count--;
			int reasonIndex = v.getReason();

			// Process a reason if it exists. 
			if (reasonIndex != -1) {
				auto& reasonClause = clauses.at(reasonIndex).getLiterals();
				for (size_t i = 1, len = reasonClause.size(); i < len; ++i) blit(reasonClause.at(i));
			}
		}
	}

	// Find the last stamped literal.
	int lprime = -1;
	do {
		lprime = trail.at(t--);
	} while (vfl(lprime).getStamp() != stamp);

	b.front() = lprime ^ 1; // Overwrite placeholder. 

	// Remove literals from the trail.
	backjump(dprime);

	// Install the new clause.
	learn(b, dprime);
}

// Remove literals from the trail until the specified level is reached.
void Solver::backjump(int dprime) {

	size_t target = levels.at(dprime + 1); // Find where the next level begins.

#ifdef DEBUG 
	std::cout << "Backjumping to d' : " << dprime << "\n";
	std::cout << "Target size is: " << target << "\n";
#endif

	// Remove elements until we are left with 'F' pointing to the next free
	// space at level dprime.
	while (trail.size() > target) {
		auto lit = trail.back();    // Get last element from trail.
		trail.pop_back();			// Remove from trail.
		auto& v = vfl(lit);			// Get variable object.
		v.setOval(v.getValue());	// Set old value to current.
		v.setValue(-1);				// Reset value.
		v.setTloc(-1);				// Reset trail location !!! Did not see in step C8!
		v.setReason(-1);			// Reset reason clause.
		if (!v.getHloc()) heap.push(&v); // Place on heap if not already there.
	}

	G = trail.size(); // G now points to the next literal to be placed on the trail. 
				      // Step C9 - 'learn' will place that next literal.

	// Set d = d'.
	levels.resize(dprime + 1);

#ifdef DEBUG
	std::cout << "Trail size now: " << trail.size() << "\n";
	std::cout << "G now: " << G << "\n";
	std::cout << "Level now: " << depth() << "\n";
#endif

}

void Solver::learn(std::vector<int>& clause, int d) {

	// We have learned another clause. Record this fact.
	incrementLearnedClauses();

	// Not unit clause - Install the clause.
	if (d) {

		int l0 = clause.front();
		int clauseNumber = clauses.size();
		clauses.push_back(Clause(clause));
		clauses.back().setClauseNumber(clauseNumber);
		addForcedLiteralToTrail(l0 , clauseNumber);

		// Ensure we are watching literals defined on level d.
		bool found = false;
		for (size_t i = 1, len = clause.size(); i < len; ++i) {
			auto& v = vfl(clause.at(i));
			auto level = v.getValue() >> 1;
			if (level == d) {
				found = true;
				std::iter_swap(clause.begin() + 1, clause.begin() + i);
				break;
			}
		}

		if (found) {
			// Set the watches for the new clause. 
			auto& v0 = vfl(l0);
			v0.addToWatch(clauseNumber, (l0 % 2) == 0);
			int l1 = clause.at(1);
			auto& v1 = vfl(l1);
			v1.addToWatch(clauseNumber, (l1 % 2) == 0);
		}
		else {
			std::cout << "No literal found on level d to watch!\n";
			std::cin.get();
			exit(1);
		}


#ifdef DEBUG
		if (clause.size() == 1) {
			std::cout << "We'd expect levels greater than 0 to not be unit clauses\n";
			std::cin.get();
			exit(1);
		}
#endif
	}
	// Unit clause - We don't install unit clauses which will be at level 0.
	else {

		// Add unit clause to trail. Unit clauses do not have reasons.
		addForcedLiteralToTrail(clause.front(), -1);

#ifdef DEBUG
		if (clause.size() != 1) {
			std::cout << "We'd expect level zero to be unit clauses\n";
			std::cin.get();
			exit(1);
		}
#endif

	}

	DEL /= rho;
}
// Add a variable to trail. The value is determined by the oval property. There is
// no reason since it was a decision.
void Solver::addDecisionVariableToTrail(int variableNumber) {

	auto& variable = vfv(variableNumber);
	if (variable.isFree()) {
		variable.setValue(static_cast<int>(depth()));
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
		variable.setValue(static_cast<int>(depth()), literal);
		variable.setTloc(static_cast<int>(trail.size()));
		variable.setReason(reason);
		trail.push_back(variable.getCurrentLiteralValue());

		// Let the clause know it is the reason for a literal. 
		if (reason != -1) clauses.at(reason).setReasonFor(variable.getVariableNumber());
	
#ifdef DEBUG 

		if (variable.getCurrentLiteralValue() != literal) {
			std::cout << "Current literal value is: " << variable.getCurrentLiteralValue() << "\n";
			std::cout << "Literal received was: " << literal << "\n";
			std::cout << "Why don't these match?\n";
			std::cin.get();
		}
		if (literal != variable.getCurrentLiteralValue()) {
			std::cout << "Looks like a bug!\n";
			std::cin.get();
			exit(1);
		}
#endif

	}

#ifdef DEBUG
	else {
		std::cout << "Placing a variable which is not free on trail!\n";
		std::cin.get();
		exit(1);
	}
#endif
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
int Solver::depth() { return levels.size() - 1; }

int Solver::getNumberOfLearnedClauses() { return numberLearnedClauses; }
void Solver::incrementLearnedClauses() { ++numberLearnedClauses; }
