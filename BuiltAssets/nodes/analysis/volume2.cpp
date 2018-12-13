#include <iostream>
#include <cmath>
#include <algorithm>
#include "outvector.h"
#include "common_math.h"

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

//progress
double prog = 0;

double i2p(int i, int cnt, int d) {
	double s = (i+0.5)/cnt;
	return (1-s)*bbox[d*2] + s*bbox[d*2+1];
}

//entry
void Do() {
	xcnt = ycnt = zcnt = reso;
	SETVEC(data, reso*reso*reso, 0);
	double v[3];
	double dp = 1. / pcnt;
	for (int p = 0; p < pcnt; p++) {
		prog = p * dp;
		for (int a = 0; a < xcnt; a++) {
			for (int b = 0; b < ycnt; b++) {
				for (int c = 0; c < zcnt; c++) {
					double center[3] = {
						i2p(a, xcnt, 0),
						i2p(b, ycnt, 1),
						i2p(c, zcnt, 2)
					};
					vec(center, positions + p*3, v);
					//periodic(v, bbox, v);
					data[a*ycnt*zcnt + b*zcnt + c] += 1/len2(v);
				}
			}
		}
	}
}