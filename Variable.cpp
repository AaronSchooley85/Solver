#include "Variable.h"


// Constructor which takes the variable number of this variable.
Variable::Variable(int v) { variable = v; }

// Set value based on the old value.
void Variable::setValue(int level) { val = 2 * level + (oval & 1); }

// Set value which depends on the literal and the level.
void Variable::setValue(int level, int literal) { val = 2 * level + (literal & 1); }

int Variable::getValue() { return val; }

// Set the HLOC to indicate heap membership status.
void Variable::setHloc(bool b) { hloc = b; }

void Variable::setTloc(int t) { tloc = t; }

// Get heap membership status.
bool Variable::getHloc() { return hloc; }

// Get the activity score used for heap location.
int Variable::getActivity() const { return act; }

bool Variable::isFree() { return val < 0; }
