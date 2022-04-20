#include "Variable.h"


// Constructor which takes the variable number of this variable.
Variable::Variable(int v) { variable = v; }

// Set value which depends on the literal and the level. TODO - literal value or boolean?
void Variable::setValue(bool value, int level) { val = 2 * level + (value & 1); }

// Set the HLOC to indicate heap membership status.
void Variable::setHloc(bool b) { hloc = b; }

// Get heap membership status.
bool Variable::getHloc() { return hloc; }

// Get the activity score used for heap location.
int Variable::getActivity() const { return act; }
