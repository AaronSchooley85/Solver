#ifndef SOLVER_H
#define SOLVER_H

#include <vector>
#include <algorithm>
#include "Heap.h"
#include "Clause.h"
#include "Variable.h"

class Solver {

	// Define a cnf as a vector of vectors.
	typedef std::vector<std::vector<int>> cnf;

	public:

		Solver(cnf);
		std::vector<bool> Solve();

	private:

		// Indices of min and max learned clauses.
		int minl = 0;
		int maxl = 0;

		// Number of variables in problem instance.
		int n = -1;

		// Index of next trail element to be processed.
		int G = 0;

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

		// rho is the damping factor used to adjust variable activities.
		double rho = 0.95;

		// DEL is the amount to increase the variable activity by.
		double DEL = 1.0;

		// Flag to indicate that we have failed to find a solution.
		bool solutionFailed = false;

		/* Private methods */

		// Add elements to trail.
		void addVariableToTrail(int v, bool b);
		void addVariableToTrail(int v);
		void addDecisionVariableToTrail(int variableNumber);
		void addForcedLiteralToTrail(int literal, int reason);

		// Select a free variable from heap and make a decision.
		void makeADecision();

		// Resolve conflicts.
		void resolveConflict(const std::vector<int>& clause);

		// See if our current variable assignments force a literal
		// to take a specific value.
		bool checkForcing(int literal);

		// Convenience functions
		Variable& vfl(int literal); // Variable object from literal.
		Variable& vfv(int variable);// Variable object from variable number.
		size_t nextStamp();
		int depth();
};

#endif