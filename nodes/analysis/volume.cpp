#include <iostream>
#include <cmath>
#include <algorithm>
#include "outvector.h"

//in pcnt 3
double* positions = 0;
//in 6
double* bbox = 0;
//in
int reso = 0;

//out xcnt ycnt zcnt
double* data = 0;
std::vector<double> _data;

//var
int pcnt = 0;

//var
int xcnt = 0;
//var
int ycnt = 0;
//var
int zcnt = 0;

int box(double x, double min, double max, int cnt) {
	double f = (x-min) / (max-min);
	int res = std::floor(f * cnt);
	if (res >= cnt) res = -1;
	return res;
}

//entry
void Do() {
	xcnt = ycnt = zcnt = reso;
	SETVEC(data, reso*reso*reso, 0);
	for (int p = 0; p < pcnt; p++) {
		int bx = box(positions[p*3], bbox[0], bbox[1], reso);
		int by = box(positions[p*3+1], bbox[2], bbox[3], reso);
		int bz = box(positions[p*3+2], bbox[4], bbox[5], reso);
		if (bx >= 0 && by >= 0 && bz >= 0)
			data[bx*ycnt*zcnt + by*zcnt + bz]++;
	}
}