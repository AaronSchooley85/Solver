#include "Clause.h"

Clause::Clause(std::vector<int> clause) {
	for (auto literal : clause) {
		literals.push_back(literal);
	}
}

std::vector<int>& Clause::getLiterals() { return literals; }