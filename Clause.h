
#ifndef CLAUSE_H
#define CLAUSE_H

#include <vector>

// TODO - For added speed the solver can have a monolithic vector
// to hold literals and instead of a vector we can have a pair of 
// iterators that hold our beginning and end in that vector. That
// will increase locality. 
class Clause {

	public:
		Clause(std::vector<int>);
		std::vector<int>& getLiterals();
		void setClauseNumber(int);
		int getClauseNumber();

	private:
		std::vector<int> literals;
		int clauseNumber = -1;

};

#endif