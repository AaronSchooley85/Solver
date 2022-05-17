#ifndef VARIABLE_H
#define VARIABLE_H

#include <vector>
#include <iostream>

//#define DEBUG

class Variable {

public:

	Variable(int variableNumber);

	// Value.
	void setValue(int level);
	void setValue(int level, int literal);
	int  getValue();

	// Access old value;
	int  getOval();
	void setOval(int);

	// Check if a literal value is true or false.
	bool isTrue(int literal);
	bool isFalse(int literal);

	// The current literal value.
	int getCurrentLiteralValue();

	// The variable number of this variable.
	int getVariableNumber();

	// The reason for the variable value.
	void setReason(int r);
	int getReason();

	// Heap location.
	void setHloc(bool b);
	bool getHloc();

	// Stamp processing.
	size_t getStamp();
	void setStamp(size_t s);

	// Add a clause number to a watch list.
	void addToWatch(int clauseNumber, bool value);
	void removeFromWatch(int clauseNumber, bool value);

	// Get all clauses which watched the variable but with
	// the opposite polarity of what it is assigned.
	std::vector<int>& getContradictedWatchers();

	// Trail location.
	void setTloc(int t);
	int getTloc();

	double getActivity() const;
	void setActivity(double amount);
	bool bumpActivity(double amount);

	bool isFree();

	// Threshold after which all variables AND DEL will be rescaled.
	const double threshold = 10.0e100;

private:

	// Clauses which watch the literal values of this variable.
	std::vector<int> watchingTrue;
	std::vector<int> watchingFalse;


	// The reason for the literal assignment. It is an index to a clause.
	int reason = -1;

	// variable attributes. 
	int variable = -1;
	size_t stamp = 0;
	int tloc = -1;
	bool hloc = false;
	int val = -1;
	int oval = 0;
	double act = 0;
};

#endif