///this is a script to test the
///ability of instant reloading

#include <iostream>
#include <cmath>

//out cnt
double* vec = 0;

//var
int cnt = 100;

double ff = 1;

//entry
void Execute() {
	if (!vec) vec = new double[100]{};
	for (int a = 0; a < cnt; a++)
		vec[a] = sinf(ff + a * 0.0314159f * 2);
	ff += 2;
}