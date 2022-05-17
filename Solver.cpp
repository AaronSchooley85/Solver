#include "Solver.h"
#include <iostream>
#include <random>
#include <chrono>
#include <unordered_set>


//#define DEBUG

// Solver constructor. Initializes variables, loads clauses, and processes
// unit clauses. 
Solver::Solver(cnf CNF, int seedArgument) {

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
			// Watch lists are not set for unit clauses.
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
					addForcedLiteralToTrail(literal, -1);
				}
				break; }

			// Standard clause of length greater than 1. 
			default:
				auto clauseNumber = clauses.size();
				clauses.push_back(Clause(encoded));
				clauses.back().setClauseNumber(clauseNumber);
				auto l0 = clauses.back().getLiterals().at(0);
				auto l1 = clauses.back().getLiterals().at(1);
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

	/*
	std::cout << "Number of variables: " << n << "\n";
	std::cout << "Number of clauses: " << clauses.size() - 1 + trail.size() << "\n";
	std::cout << "Number of unit clauses: " << trail.size() << "\n";
	*/

	// Add free variables to heap.
	std::vector<Variable*> shuffledVariablePointers;
	for (int i = 1; i < variables.size(); ++i) shuffledVariablePointers.push_back(&variables.at(i));

	// Get our random generator seed. Use supplied argument if provided, else generate a random seed.
	int seed;
	if (seedArgument < 0) seed = std::chrono::system_clock::now().time_since_epoch().count();
	else seed = seedArgument;

	// Give seed to heap as well.
	heap.setSeed(seed);

	// Shuffle the variables to add to the heap. This prevents getting stuck in "ruts" if invoked multiple times.
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


			// If we are finished. I.e the number of variables on the trail
			// is equal to the number of variables in the problem.
			if (trail.size() == n) {
				
				// If we're doing a full run, check if we actually encountered any conflicts.
				int max = 0;
				if (fullRun) max = static_cast<int>(*std::max_element(conflicts.begin(), conflicts.end()));

				// If not a full run we genuinely solved the problem.
				// Or if it's a full run but we did not encounter any conflicts.
				if (!fullRun || ( fullRun && (max == 0))) {

					// Construct and return boolean vector.
					std::vector<bool> solution(n + 1);
					solution.front() = true;
					for (auto t : trail) solution.at(t >> 1) = t & 1 ? false : true;
					return solution;
				}
				// Otherwise we "succeeded" because we ignored conflicts for a full run.
				// Beging resolving conflicts and purging useless clauses.
				else {
					fullRun = false;
					//std::cout << "Full run finished\n";
					purgeProcessing();
					continue;
				}
			}
			// Check if it's time to get rid of useless learned clauses.
			else if (!fullRun && totalLearnedClauses > purgeThreshold) {
				fullRun = true;
				//std::cout << "Learned " << numberLearnedClauses << ". Full run starting\n";
				for (auto& c : conflicts) c = 0;
			}
			// Is it time to flush literals?
			else if (totalLearnedClauses >= flushThreshold) {
				flushProcessing();
			}

			// Not finished. We need to make a decision.
			// Select a free variable from the heap and place on trail.
			// Will result in F = G + 1. i.e. will increment F.
			makeADecision();
			
		}

		// Process next literal on trail pointed at by G and increment G.
		// If a conflict occurs, a new literal will be placed on the trail and
		// G will be pointing to it already. Immediately take from trail.
		bool conflictEncountered = false;
		do {
			auto literal = trail.at(G++);
			conflictEncountered = checkForcing(literal);
			if (conflictEncountered && solutionFailed) return std::vector<bool>{false};
		} while (conflictEncountered);
	}

}

// Knuth step C4.
// Check if the literal being processed forces other literals to take values or falsifies
// a clause. If so, resolve that conflict. 
// Return value is status of conflict. True -> conflict detected. False -> no conflict.
bool Solver::checkForcing(int literal) {

#ifdef DEBUG
	std::cout << "Processing literal " << literal << "\n";
#endif

	// Get the variable object corresponding to the literal.
	auto& variable = vfl(literal);

	// Get the complement of the literal. 
	int contradictedLiteral = literal ^ 1;

	// Get indices of each clause which watches the contradicted polarity of the literal.
	auto& contradictedWatchers = variable.getContradictedWatchers();

	// Create a copy to iterate through. This is so we don't modify the same collection we are iterating through.
	std::vector<int> contradictedWatcherIndicesToProcess(contradictedWatchers.begin(), contradictedWatchers.end());

	// Inspect each contradicted clause. Below we remove any if necessary from variable's collection.
	for (auto contradictedClauseNumber : contradictedWatcherIndicesToProcess){

		// Using the clause index, get the clause object which contains the contradicted literal.
		auto& contradictedClause = clauses.at(contradictedClauseNumber);

		// Get the literals which comprise this clause.
		auto& contradictedClauseLiterals = contradictedClause.getLiterals();

		// Algorithm C requires the literal at index 1 of the clause to be the contradicted literal.
		// Swap first two literals if the element at index 1 is not the contradicted literal.
		// This is a branchless swap which has proven to be a bit faster. 
		bool swap = contradictedClauseLiterals.at(1) != contradictedLiteral;
		std::iter_swap(contradictedClauseLiterals.begin(), contradictedClauseLiterals.begin() + swap);

#ifdef DEBUG
		// Sanity check.
		if (contradictedClauseLiterals.at(1) != contradictedLiteral) {
			std::cout << "At this point l1 should be the complement of the selected literal.\n";
		std::cout << "Contradicted clause number: " << contradictedClauseNumber << "\n";
		std::cout << "Contradicted clause literals: ";
			printVector(contradictedClauseLiterals);
			std::cin.get();
		}

		std::cout << "Contradicted clause number: " << contradictedClauseNumber << "\n";
		std::cout << "Contradicted clause literals: ";
		printVector(contradictedClauseLiterals);
#endif

		// If literal at index 0 is not true (i.e. false or unset).
		auto l0 = contradictedClauseLiterals.front();
		auto& v0 = vfl(l0);

		// If the first literal at index 0 is true, nothing must be done.
		if (!v0.isTrue(l0)) {
			
			// We will try to swap the literal at index 1 with another which is NOT FALSE.
			bool swapSuccess = false;
			for (size_t i = 2, n = contradictedClauseLiterals.size(); i < n; ++i) {

				// Get the candidate literal and its associated variable object.
				auto lx = contradictedClauseLiterals.at(i);
				auto& vx = vfl(lx);

				// If the new variable has not been set false.
				if (!vx.isFalse(lx)) {

#ifdef DEBUG
					std::cout << "Swapping l1 with " << lx << "\n";
#endif

					// Swap elements, add clause to new watched variable, and remove from old variable's watch.
					std::swap(contradictedClauseLiterals.at(1), contradictedClauseLiterals.at(i));
					vx.addToWatch(contradictedClauseNumber, !(lx & 1));
					variable.removeFromWatch(contradictedClauseNumber, !(contradictedLiteral & 1));
					swapSuccess = true;
					break;
				}
			}

			// If we could not swap with another variable, we must check literal at clause index 0.
			// Our only hope is that it is free and can therefore be set true.
			if (!swapSuccess) {

				if (v0.isFree()) {
#ifdef DEBUG
					std::cout << "Could not swap. Adding " << l0 << " to trail.\n";
#endif
					addForcedLiteralToTrail(l0, contradictedClauseNumber);
				}
				// We must resolve a conflict.
				else {
#ifdef DEBUG
					std::cout << "Could not swap. Resolving conflict.\n";
#endif
					// If there have been no decision levels created, we failed.
					if (depth() == 0) {
						solutionFailed = true;
						return true;
					}
					// We resolve conflicts if we are not at level 0 and not doing a full run.
					else if (!fullRun) {

						// Learn a new clause and return the depth we must return to for installation.
						int dprime = resolveConflict(contradictedClauseLiterals);

						// Remove literals from the trail.
						backjump(dprime);

						// Shorted the learned clause for efficiency.
						removeRedundantLiterals();

						// Install the new clause.
						learn(dprime);

						return true;
					}
					// On full runs we just ignore the conflict and move on.
					else {
						// Record first conflict for this level.
						int d = depth();
						if (conflicts.at(d) == 0) conflicts.at(d) = contradictedClauseNumber;
						//return false; // !!!!!!! SHOULD THIS BE HERE?
					}

				}
			}
		}
#ifdef DEBUG
		else
		{
			std::cout << "l0 true, doing nothing.\n";
		}
#endif
	}

	return false;
}

// Knuth step C6.
void Solver::makeADecision() {

	// Record trail index at which this level begins.
	levels.push_back(trail.size());

	// Should this be done here?
	// Make sure there's a spot in LS for every level.
	while (LS.size() <= depth()) pushLevelStamp(0); 

	// Ensure we have a large enough conflicts vector.
	// Will be zeroed at the beginning of each full run.
	while (conflicts.size() <= depth()) conflicts.push_back(0);

#ifdef DEBUG
	std::cout << "########## " << "LEVEL " << depth() << " ###########\n";
#endif

	// Repair heap if necessary.
	if (heapCorrupted) {
		heap.reheapify();
		heapCorrupted = false;
	}

	// Repeatedly pop a variable from the heap until we find one
	// that is free.
	Variable* nextFree = nullptr;
	do {
		nextFree = heap.pop(true); // Get max element but allow for occassional random elements. 
	} while (!(nextFree->isFree()));
	
	// Add the new decision variable to the trail.
	// This will cause F = G + 1
	addDecisionVariableToTrail(nextFree->getVariableNumber());
}

// Construct a new clause.
int Solver::resolveConflict(const std::vector<int>& clause, int d) {
	
#ifdef DEBUG
	std::cout << "Trail: ";
	if (checkVectorForDuplicates(trail)) {
		std::cout << "ah ha!";
		std::cin.get();
	}
	for (auto& t : trail) {
		std::cout << t << "(" << vfl(t).getTloc() << ", " << (vfl(t).getValue() >> 1) << "), ";
	}
	std::cout << "\n";
	std::cout << "Levels: ";
	printVector(levels);

	std::cout << "Conflict encountered at level " << depth() << "\n";
#endif

	// Data needed for blit routine below.
	int count = 0;
	b.clear();
	b.push_back(-1);
	incrementStamp();
	int dprime = 0;
	int learnCount = clauses.size() - minl;

	// Process the first literal in the clause.
	auto l0 = clause.front();
	auto& v0 = vfl(l0);
	v0.setStamp(stamp); // Stamped here, not in 'blit' so count is not affected. 
	bool rescale = false;
	rescale |= v0.bumpActivity(DEL);
	heapCorrupted = true; // !! This makes a huge improvement. Heap corruption is a serious issue. 

	// If depth was specified use that depth, otherwise get it from level vector size.
	int currentDepth = d < 0 ? depth() : d;

	// Do we need to zero the LS array before blit runs? !!!!!!!!!!!!!!!!!!!!!!!!!!
	for (auto& x : LS) x = 0;

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
					b.push_back(v.getCurrentLiteralValue() ^ 1);
					dprime = std::max(p, dprime);
					int levelStamp = getLevelStamp(p);
					if (levelStamp <= stamp) setLevelStamp(p, stamp + (levelStamp == stamp));
				}
			}
		}
	};

	// Apply blit algorithm to all literals at index GREATER THAN 0 in clause.
	for (size_t i = 1, len = clause.size(); i < len; ++i) blit(clause.at(i));


	// Get the highest trail index of ALL literals in the clause.
	int t = 0;
	for (auto lit : clause) {
		auto& v = vfl(lit);
		t = std::max(v.getTloc(), t); // Am I using 'max' correctly?
	}

#ifdef DEBUG
	std::cout << "Max T: " << t << "\n";
#endif

	while (count > 0) {
		auto l = trail.at(t--); // Get literal furthest up the trail.
		auto& v = vfl(l);
		if (v.getStamp() == stamp) {

			count--;
			int reasonIndex = v.getReason();

			// Process a reason if it exists. 
			if (reasonIndex != -1) {

				// This clause is participating in a resolution. Increase its activity. !!!!! NOT ENTIRELY SURE WHERE TO PUT THIS.
				auto clauseActivity = clauses.at(reasonIndex).getActivity();
				clauses.at(reasonIndex).setActivity(clauseActivity + std::pow(clauseRho, -learnCount));

				// Blit literals at index greater than 0.
				auto& reasonClause = clauses.at(reasonIndex).getLiterals();
				for (size_t i = 1, len = reasonClause.size(); i < len; ++i) blit(reasonClause.at(i));
			}
		}
	}

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

#ifdef DEBUG
	// dprime is where we will jump back to. It needs to be less than current depth.
	if (dprime >= currentDepth) {
		std::cout << "d' is supposed to be less than depth\n";
		std::cin.get();
	}
#endif

	// Find the final stamped literal.
	int lprime = -1;
	do {
		lprime = trail.at(t--);
	} while (vfl(lprime).getStamp() != stamp);

#ifdef DEBUG
	if (dprime >= currentDepth) {
		std::cout << "d' should be less than the learned clause literal l0.\n";
		std::cin.get();
	}
	else {
		std::cout << "Learned clause l0 is " << (lprime ^ 1) << " at level " << (vfl(lprime).getValue() >> 1) << "\n";
		std::cout << "d' was found to be " << dprime << "\n";
	}
#endif
	b.front() = lprime ^ 1; // Overwrite placeholder. 

#ifdef DEBUG
	std::cout << "Learned clause: ";
	printVector(b);
	std::cout << "\n";
#endif

	// Return the level we must return to for learned clause installation.
	return dprime;
}


// Improve processing speed by removing redundant clauses.
void Solver::removeRedundantLiterals() {

	// The learned clause is stored in the member variable 'b'.
	auto& clause = b;

	int i = 1;
	while (i < clause.size()) {

		int bi = clause.at(i);
		int level = vfl(bi).getValue() >> 1;
		int levelStamp = getLevelStamp(level);

		// Redundant.                        bi or bi ^ 1 ? 
		if (levelStamp == (stamp + 1) && red(bi, stamp)) {

			std::iter_swap(clause.begin() + i, clause.end() - 1);
			clause.pop_back();

			// Don't increment index. Swap placed next literal at index i.
		}
		// Not redundant. Incrment index.
		else {
			++i;
		}
	}
}

bool Solver::red(int lit, size_t stamp){

	auto& v0 = vfl(lit);

	// If l is a decision literal, return false.
	int reasonIndex = v0.getReason();
	if (reasonIndex == -1) return false;

	// Get the literals which comprise the clause.
	auto& reasonLiterals = clauses.at(reasonIndex).getLiterals();

	// Iterate through all elements except the first.
	for (size_t i = 1, len = reasonLiterals.size(); i < len; ++i) {

		int l = reasonLiterals.at(i);
		auto& v = vfl(l);
		int level = v.getValue() >> 1;
		if (level > 0) {

			int vstamp = v.getStamp();
			if (vstamp == (stamp + 2)) return false;                       // l or l^ 1 ?
			else if (vstamp < stamp && ((getLevelStamp(level) < stamp) || !red(l, stamp))) {
				v.setStamp(stamp + 2);
				return false;
			}
			else {
				//v0.setStamp(stamp + 1);
				//return true;
			}
		}

		//v0.setStamp(stamp + 1);
		//return true;
	}

	v0.setStamp(stamp + 1);
	return true;
}


int Solver::getLevelStamp(int index) { return LS.at(index); }

void Solver::setLevelStamp(int index, int value) { LS.at(index) = value; }

void Solver::pushLevelStamp(int value) { LS.push_back(value); }

void Solver::popLevelStamp() { LS.pop_back(); }

// Remove literals from the trail until the specified level is reached.
void Solver::backjump(int dprime) {

	size_t target = levels.at(dprime + 1); // Find where the next level begins.

#ifdef DEBUG 
	std::cout << "Backjumping to d' : " << dprime << "\n";
	std::cout << "Current trail size is: " << trail.size() << "\n";
	std::cout << "Target trail size is: " << target << "\n";
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
		if (v.getReason() != -1) clauses.at(v.getReason()).setReasonFor(-1);
		v.setReason(-1);			// Reset reason clause.
		if (!v.getHloc()) heap.push(&v); // Place on heap if not already there.
	}

	G = trail.size(); // G now points to the next literal to be placed on the trail. 
				      // Step C9 - 'learn' will place that next literal.

	levels.resize(dprime + 1);

#ifdef DEBUG
	for (auto t : trail) {
		if ((vfl(t).getValue() >> 1) > dprime) {
			std::cout << "Literal on trail has depth greater than current depth\n";
			std::cin.get();
		}
	}
#endif

#ifdef DEBUG
	std::cout << "Trail size now: " << trail.size() << "\n";
	std::cout << "G now: " << G << "\n";
	std::cout << "Level now: " << depth() << "\n";
#endif

}

void Solver::learn(int dprime) {

	totalLearnedClauses++;

	// Access learned clause from member variable 'b'.
	auto& clause = b;

	// Not unit clause - Install the clause.
	if (dprime) {

		int l0 = clause.front();

		// Ensure we are watching literals defined on level d.
		bool found = false;
		for (size_t i = 1, len = clause.size(); i < len; ++i) {
			auto& v = vfl(clause.at(i));
			auto level = v.getValue() >> 1;
			if (level == dprime) {
				found = true;
				std::iter_swap(clause.begin() + 1, clause.begin() + i);
				break;
			}
		}

		int clauseNumber = clauses.size();
		clauses.push_back(Clause(clause));
		clauses.back().setClauseNumber(clauseNumber);
		addForcedLiteralToTrail(l0 , clauseNumber);

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
		agility = agility - (agility >> 13) + (((variable.getOval() - variable.getValue()) & 1) << 19);
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
		agility = agility - (agility >> 13) + (((variable.getOval() - variable.getValue()) & 1) << 19);
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
		std::cout << "Attempted to place literal " << literal << " but its already at TLOC: " << variable.getTloc() << "\n";
		std::cin.get();
		exit(1);
	}
#endif
}


// Convenience methods to get variable objects.
Variable& Solver::vfl(int literal) { return variables.at(literal >> 1); }
Variable& Solver::vfv(int variableNumber) { return variables.at(variableNumber); }

void Solver::incrementStamp() {
	stamp += 3;
}

// Index of levels is the same as level. So level 1 begins
// at index 1. Level 0 at index zero is always equal to zero
// and doesn't count toward the level count. Therefore with only
// zero in index zero, we have a depth of zero (size - 1).
int Solver::depth() { return levels.size() - 1; }

void Solver::purgeProcessing() {

	// Initialize minimum to largest possible value.
	int minDprime = INT32_MAX;

	std::vector<std::vector<int>> clausesToInstall;

	// Visit conflicts in reverse order.
	for (auto i = conflicts.rbegin(); i != conflicts.rend(); i++){
		
		// Get the depth which is the index of the vector.
		auto d = std::distance(i, conflicts.rend()) -1;

		// Obtain the value pointed to by the iterator.
		int conflictClauseIndex = *i;

		// If a conflict clause was recorded at depth 'd'.  
		if (conflictClauseIndex > 0) {

			auto& conflictClause = clauses.at(conflictClauseIndex);
			auto& conflictClauseLiterals = conflictClause.getLiterals();

			// Resolved conflict is stored in 'b' vector.
			int dprime = resolveConflict(conflictClauseLiterals, d);
			removeRedundantLiterals();

			// If new minimum, record it and restart install vector.
			if (dprime < minDprime) {
				clausesToInstall.clear();
				minDprime = dprime;
			}

			// Save the 'b' vector for installation later.
			if (dprime == minDprime) clausesToInstall.push_back(b);
		}
	}

	// Install all literals defined at minimum dprime.
	backjump(minDprime);

	// Loop through each clause which must be installed.
	for (auto clauseToInstall : clausesToInstall) {

		// Function 'learn' depends on this class' 'b vector' 
		// containing the clause. Load it first.
		b = clauseToInstall;
		learn(minDprime);
	}

	// Calculate range scores for all learned clauses.
	for (auto& x : LS) x = 0;
	std::vector<int> m(256, 0);
	for (int c = minl; c < clauses.size(); ++c) {
		auto& clause = clauses.at(c);

		// If this clause is a reason for a literal it gets a score of zero.
		if (clause.getReasonFor() != -1) clause.setRange(0);
		else {

			int p = 0, r = 0;
			for (auto lit : clause.getLiterals()) {

				auto& v = vfl(lit);
				auto val = v.getValue();
				auto level = val >> 1;

				// If it was set on level 0. 
				if (level == 0 && v.isTrue(lit)) {
					clause.setRange(256);
					break;
				}
				else if (level >= 1 && LS[level] < c) {
					LS[level] = c;
					++r;
				}
				else if (level >= 1 && LS[level] == c && v.isTrue(lit)) {
					LS[level] = c + 1;
					++p;
				}
			}
			int a = (int) std::floor(16.0 * (p + clauseAlpha * (r - p)));
			r = (int) std::min(a, 255);
			m.at(r)++;
			clause.setRange(r);
		}
	}
	
	// Remove some learned clauses.
	int T = (clauses.size() - minl) / 2; // Number of learned clauses to retain.

	// Find the number of elements of vector 'm' needed such that
	// their sum.
	int sum = 0;
	int j = 0;
	while (sum <= T) sum += m.at(j++);
	sum -= m.at(j - 1);
	int tieBreakers = T - sum;

	// Purge clauses whose range is greater than or equal to j.
	// Book says do in order. Am I allowed to swap and pop?
	for (int i = minl; i < clauses.size();) {
		
		// Get the clause at this index.
		auto& proposedClause = clauses.at(i);

		// Purge if range score too large.
		if (proposedClause.getRange() >= j) {

			// Swap clause to delete with last clause.
			std::iter_swap(clauses.begin() + i, clauses.end() - 1);

			// Update moved clause so that it has the new clause number and
			// inform the watched variables that the number is changed.
			auto& newClause = clauses.at(i);
			int originalClauseNumber = newClause.getClauseNumber();

			auto& literals = newClause.getLiterals();
			auto wl0 = literals.at(0);
			auto wl1 = literals.at(1);
			auto& wv0 = vfl(wl0);
			auto& wv1 = vfl(wl1);

			// Inform the watched variables that the clause number has changed.
			wv0.removeFromWatch(originalClauseNumber, !(wl0 & 1));
			wv1.removeFromWatch(originalClauseNumber, !(wl1 & 1));
			wv0.addToWatch(i, !(wl0 & 1));
			wv1.addToWatch(i, !(wl1 & 1));

			newClause.setClauseNumber(i); // Inform clause of its new clause number.

			// If the moved clause is a reason for a literal, update the reason property.
			auto reasonFor = newClause.getReasonFor();
			if (reasonFor != -1) variables.at(reasonFor).setReason(i);
			
			auto& removedClause = clauses.back();
			auto& removedClauseLiterals = removedClause.getLiterals();
			int rl0 = removedClauseLiterals.at(0);
			int rl1 = removedClauseLiterals.at(1);
			auto& rv0 = vfl(rl0);
			auto& rv1 = vfl(rl1);
			rv0.removeFromWatch(i, !(rl0 & 1));
			rv1.removeFromWatch(i, !(rl1 & 1));

			if (removedClause.getReasonFor() != -1) {
				std::cout << "Fatal error! Removed clause should not be a reason for any literal!\n";
				std::cin.get();
			}
			// Processing complete. Remove clause.
			clauses.pop_back();

			// Don't increment i. Swap has placed next value at current index.
		}
		else {
			i++;
		}
	}

	capDelta += lowerDelta;
	purgeThreshold += capDelta;
}

// Flush literals from the trail if heuristics determine it is advised. 
// Otherwise return immediately. 
void Solver::flushProcessing() {


	flushThreshold += vf;
	if ((uf & -uf) == vf) {
		uf++;
		vf = 1;
		thetaF = std::pow(2, 32) * psi;
	}
	else {
		vf *= 2;
		thetaF += (thetaF >> 4);
	}

	/*auto a = agility / std::pow(2, 32);

	// Below we return if it's not time to flush. 
	switch (flushCapDelta) {
	case 1:
		if (a > std::pow(theta, 0) * psi) return;
	case 2:
		if (a > std::pow(theta, 1) * psi) return;
	case 3:
		if (a > std::pow(theta, 2) * psi) return;
	case 4:
		if (a > std::pow(theta, 3) * psi) return;
	case 5:
		if (a > std::pow(theta, 4) * psi) return;
	case 6:
		if (a > std::pow(theta, 5) * psi) return;
	case 7:
		if (a > std::pow(theta, 6) * psi) return;
	case 8:
		if (a > std::pow(theta, 7) * psi) return;
	case 9:
		if (a > std::pow(theta, 8) * psi) return;
	case 10:
		if (a > std::pow(theta, 9) * psi) return;
	case 11:
		if (a > std::pow(theta, 10) * psi) return;
	case 12:
		if (a > std::pow(theta, 11) * psi) return;
	case 13:
		if (a > std::pow(theta, 12) * psi) return;
	case 14:
		if (a > std::pow(theta, 13) * psi) return;
	case 15:
		if (a > std::pow(theta, 14) * psi) return;
	} */

	if (agility <= thetaF) {
		auto maxActivity = heap.queryMaxFreeVariable()->getActivity();

		// !!!!!!!!!!!! Knuth's book does not mention any check for dprime not exceeding levels. Why does my code need it?
		int dprime = 0;
		while (dprime < (levels.size() -1) && vfl(trail.at(levels.at(dprime + 1))).getActivity() >= maxActivity) dprime++;
		if (dprime < depth()) {
			backjump(dprime);
		}
	}
}

bool Solver::checkVectorForDuplicates(std::vector<int>& v) {

	for (size_t i = 0, len = v.size(); i < len; ++i) {
		for (size_t j = i + 1; j < len; ++j) {
			if (v[j] == v[i] || v[j] == (v[i] ^ 1)) return true;
		}
	}
	return false;
}

void Solver::printVector(std::vector<int>& v) {
	
	for (int i = 0; i < v.size(); ++i) {
		std::cout << v[i];
		if (i < (v.size() - 1)) std::cout << ", ";
	}
	std::cout << "\n";
}

/* 

	TODO: 

	1.) Is ACT corrupted during blit? Do we need to reheapify afterward?
	5.) Rescaling happening correctly?
	6.) ACT accumulation being handled correctly?
	7.) What if the CNF is missing variable numbers? I.e variables 1,2, and 4. Does missing 3 affect us?
		- If so, must we remap the inputs to a sequential series?
	8.) Profile performance when switching to branchless operations when possible.
*/
