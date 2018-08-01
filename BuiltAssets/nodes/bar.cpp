///this is a script to test the
///ability of instant reloading

#include <iostream>
#include <cmath>

VECSZ(cnt)
VAROUT float* vec = 0;

VECVAR int cnt = 100;

float ff = 1;

ENTRY Execute() {
	if (!vec) vec = new float[100]{};
	for (int a = 0; a < cnt; a++)
		vec[a] = sinf(ff + a * 0.0314159f * 2);
	ff += 2;
}