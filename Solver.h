#ifndef SOLVER_H
#define SOLVER_H

#include <vector>
#include <math.h>
#include <algorithm>
#include "Heap.h"
#include "Clause.h"
#include "Variable.h"
#include <unordered_map>


class Solver {

	// Define a cnf as a vector of vectors.
	typedef std::vector<std::vector<int>> cnf;

	public:

		Solver(cnf, int seedArgument = -1);
		std::vector<bool> Solve();

	private:

		// Indices of min and max learned clauses.
		int minl = 0;
		int maxl = 0;
		int totalLearnedClauses = 0;

		// Number of variables in problem instance.
		int n = -1;

		// Index of next trail element to be processed.
		int G = 0;

		// Used by bimp table processing.
		int E = 0;

		// Unique stamp value.
		size_t stamp = 0;

		// Heap to hold our free variables, sorted by activity.
		Heap heap;

		// Variables and clauses.
		std::vector<Clause> clauses;
		std::vector<Variable> variables;
		// Trail of literals and a record of the levels.
		std::vector<int> trail; // F = trail.size()
		std::vector<int> levels;
		std::unordered_map<int, std::vector<int>> bimp;

		// Reusable vector for temporarily holding learned clauses.
		std::vector<int> b;

		// Level stamps.
		std::vector<int> LS;

		// Threshold at which we purge useless learned clauses.
		int capDelta = 1000;
		int lowerDelta = 500;
		int purgeThreshold = capDelta;

		// Flushing parameters.
		double theta = 17.0 / 16;
		double psi = 0.05;
		int flushThreshold = 1;
		int uf = 1;
		int vf = 1;
		uint64_t thetaF = 1;

		int32_t agility = 0;

		// Flag indicating "full runs" being performed.
		bool fullRun = false;

		// Records the first conflict encountered at each level during full runs.
		std::vector<int> conflicts;

		// rho is the damping factor used to adjust variable activities.
		double rho = 0.9; // For some reason much higher values than the book work well for my test cases. 

		// DEL is the amount to increase the variable activity by.
		double DEL = 1.0;

		// Used for clause range calculation.
		double clauseAlpha = 0.4;
		double clauseRho = 0.9995;

		// Flag to indicate that we have failed to find a solution.
		bool solutionFailed = false;

		// Set when the 'blit' algorithm bumps activity scores.
		bool heapCorrupted = false;


		/* Private methods */

		// Add elements to trail.
		void addDecisionVariableToTrail(int variableNumber);
		void addForcedLiteralToTrail(int literal, int reason);

		// Used for the bimp table.
		bool bimpProcessing(int l0);
		std::vector<int> takeAccountOf(int l0);

		// High level conflict handling procedure.
		void conflictProcessing(std::vector<int>&);

		// Modify LS vector.
		void pushLevelStamp(int value);
		void popLevelStamp();
		int getLevelStamp(int index);
		void setLevelStamp(int index, int value);

		// Select a free variable from heap and make a decision.
		void makeADecision();

		// See if our current variable assignments force a literal to take a specific value.
		bool checkForcing(int literal);

		// Resolve conflicts which are encountered by force checking.
		int resolveConflict(const std::vector<int>& clause, int depth = -1);

		// Shorten clauses by removing redundancy. 
		void removeRedundantLiterals();
		bool red(int literal, size_t stamp);

		// Remove literals from the trail until the specified level is reached.
		void backjump(int dprime);

		// Install the newly learned clause from conflict resolution.
		void learn(int dprime);
		
		// Resolve backlogged conflicts and calculate range scores.
		void purgeProcessing();

		// If agility scores dictate it, flush some literals from the trail.
		void flushProcessing();

		// Diagnostic method for checking for duplicates in vector.
		bool checkVectorForDuplicates(std::vector<int>&);

		void printVector(std::vector<int>&);

		// Convenience functions
		Variable& vfl(int literal); // Variable object from literal.
		Variable& vfv(int variable);// Variable object from variable number.
		void incrementStamp();
		int depth();
};

#endif