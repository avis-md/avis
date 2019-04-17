#pragma once

/*! RNG System
[av] */
class Random {
public:
	static void Seed(uint i);
	static float Value(), Range(float min, float max);
};