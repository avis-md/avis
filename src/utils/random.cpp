#include "Engine.h"

void Random::Seed(uint i) {
	srand(i);
}

float Random::Value() {
	return ((float)rand()) / ((float)RAND_MAX);
}

float Random::Range(float min, float max) {
	return min + Random::Value()*(max - min);
}