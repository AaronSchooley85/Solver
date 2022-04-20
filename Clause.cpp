#include "Clause.h"

// Load literals. This solver used the encoding that any integer k is
// encoded as 2k for even k and 2k + 1 otherwise.
Clause::Clause(std::vector<int> clause) {
	for (auto literal : clause) {
		literals.push_back(2 * std::abs(literal) + (literal < 0));
	}
}