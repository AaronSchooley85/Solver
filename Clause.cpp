#include "Clause.h"

Clause::Clause(std::vector<int> clause) {
	for (auto literal : clause) {
		literals.push_back(literal);
	}
}

std::vector<int>& Clause::getLiterals() { return literals; }

void Clause::setClauseNumber(int c) { clauseNumber = c; }

int Clause::getClauseNumber() { return clauseNumber; }

int Clause::getReasonFor() { return reasonFor; }

void Clause::setReasonFor(int variableNumber) { reasonFor = variableNumber; }

int Clause::getRange() { return range; }

void Clause::setRange(int r) { range = r; }

void Clause::setActivity(double a) { activity = a; }

double Clause::getActivity() { return activity; }