#include"goatlings.h"
Goatling::Goatling() {
	state = ALIVE;
}
void Goatling::setState(status state) {
	if (state == DEAD && this->state == ALIVE)
		nDead++;
	if (state == ALIVE && this->state == DEAD)
		nDead--;
	this->state = state;
}

status Goatling::getState() { return state; }

int Goatling::getNumber() {
		if (state == ALIVE)
			return rand() % 100 + 1;
		return rand() % 50 + 1;
}
Goatling::~Goatling() {}