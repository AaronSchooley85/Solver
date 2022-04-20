#ifndef VARIABLE_H
#define VARIABLE_H

#include <vector>

class Variable {

public:

	Variable(int variableNumber);

	void setValue(bool value, int level);
	void getValue();

	void setHloc(bool b);
	bool getHloc();

	int getActivity() const;

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