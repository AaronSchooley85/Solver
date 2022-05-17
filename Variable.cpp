#include "Variable.h"


// Constructor which takes the variable number of this variable.
Variable::Variable(int v) { variable = v; }

// Set value based on the old value.
void Variable::setValue(int level) { val = 2 * level + (oval & 1); }

// Set value which depends on the literal and the level.
void Variable::setValue(int level, int literal) { val = 2 * level + (literal & 1); }

// Get the current literal value determined by variable number and old value.
int Variable::getCurrentLiteralValue() { return 2 * variable + (val & 1); }

// Get the variable number of this variable object. This is its index in the variable vector.
int Variable::getVariableNumber() { return variable; }

int Variable::getValue() { return val; }

// Access old value.
int Variable::getOval(){ return oval; }
void Variable::setOval(int v) { oval = v; }

// Check the truth value of a literal.
bool Variable::isTrue(int literal) { return ((val >= 0) && ((val + literal) % 2 == 0)); }
bool Variable::isFalse(int literal) { return ((val >=0) && ((val + literal) % 2 != 0)); }

std::vector<int>& Variable::getContradictedWatchers(){ return (val & 1) ? watchingTrue : watchingFalse; }

// Access the reason property.
int Variable::getReason() { return reason; }
void Variable::setReason(int r) { reason = r; }

// Set the HLOC to indicate heap membership status.
void Variable::setHloc(bool b) { hloc = b; }

// The trail index of the variable.
void Variable::setTloc(int t) { tloc = t; }
int  Variable::getTloc() { return tloc; }

// Get heap membership status.
bool Variable::getHloc() { return hloc; }

// Get the activity score used for heap location.
double Variable::getActivity() const { return act; }

// Set the activity to a specific amount. Typically used for rescaling.
void Variable::setActivity(double amount) { act = amount; }

// Increase the activity. A boolean flag is returned to indicate to the
// calling routine whether the threshold has been exceeded and must be rescaled. 
bool Variable::bumpActivity(double amount) { 
	act += amount; 
#ifdef DEBUG
	if (act > threshold) std::cout << "Rescaling needed\n";
#endif
	return act > threshold;
}

bool Variable::isFree() { return val < 0; }

// Add a clause number to the correct watch list.
void Variable::addToWatch(int clauseNumber, bool value) {
	if (value) watchingTrue.push_back(clauseNumber);
	else watchingFalse.push_back(clauseNumber);
}

void Variable::removeFromWatch(int clauseNumber, bool value) {

	auto& contradictedWatchers = value ? watchingTrue : watchingFalse;

	// Remove clause number from watch list of previously watched variable. Exchange and pop off back.
	auto indexToRemove = std::find(contradictedWatchers.begin(), contradictedWatchers.end(), clauseNumber);
	if (indexToRemove != contradictedWatchers.end()) {

		std::iter_swap(indexToRemove, contradictedWatchers.end() -1);
		contradictedWatchers.pop_back();
	}
	else {
		std::cout << "Fatal bug. Did not find the clause number to remove.\n";
		std::cin.get();
	}
}

// Getter and setter for stamp value.
size_t Variable::getStamp() { return stamp; }
void Variable::setStamp(size_t s) { stamp = s; }
