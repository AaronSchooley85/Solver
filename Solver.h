#ifndef SOLVER_H
#define SOLVER_H

#include <vector>
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

		// Index of next trail element to be processed.
		int G = 0;

		// Heap to hold our free variables, sorted by activity.
		Heap heap;

		// Variables and clauses.
		std::vector<Clause> clauses;
		std::vector<Variable> variables; // Length of this vector is the number of variables in problem.

		// Trail of literals and a record of the levels.
		std::vector<int> trail; // F = trail.size()
		std::vector<int> levels;

		// rho is the damping factor used to adjust variable activities.
		double rho = 0.95;

		/* Private methods */

		// Add elements to trail.
		void addVariableToTrail(int v, bool b);
		void addVariableToTrail(int v);
		void addDecisionVariableToTrail(int variableNumber);
		void addForcedLiteralToTrail(int literal, int reason);

		// Convenience functions
		Variable& vfl(int literal); // Variable object from literal.
		Variable& vfv(int variable);// Variable object from variable number.
};

#endif