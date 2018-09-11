#include <iostream>
#include <cmath>

//in cnt
double* ivec = 0;
//out cnt
double* ovec = 0;
//var
int cnt = 0;

//entry
void Execute() {
	if (ovec) delete[](ovec);
	ovec = new double[cnt]{};
	for (int a = 0; a < cnt; a++)
		ovec[a] = ivec[a] * std::exp(-a*2.0/cnt);
	printf("C says hello!");
}