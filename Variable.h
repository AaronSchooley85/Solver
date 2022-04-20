#ifndef VARIABLE_H
#define VARIABLE_H

#include <vector>

class Variable {

public:

	Variable(int variableNumber);

	// Value.
	void setValue(int level);
	void setValue(int level, int literal);
	int  getValue();

	// Heap location.
	void setHloc(bool b);
	bool getHloc();

	// Trail location.
	void setTloc(int t);

	int getActivity() const;

	bool isFree();

private:

	// Clauses which watch the literal values of this variable.
	std::vector<int> watchingTrue;
	std::vector<int> watchingFalse;


	// The reason for the literal assignment. It is an index to a clause.
	int reason = -1;

	// variable attributes. 
	int variable = -1;
	int stamp = 0;
	int tloc = -1;
	bool hloc = false;
	int val = -1;
	int oval = -1;
	int act = 0;

};

#endif