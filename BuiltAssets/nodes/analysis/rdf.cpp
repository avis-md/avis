#include <iostream>
#include <vector>
#include "common_math.h"

//in pcnt 3
double* positions = 0;
//in 6
double* bbox = 0;
//in
int count = 0;
//in
double cutoff = 0;

//out count
double* rdf = 0;
std::vector<double> _rdf;

//var
int pcnt = 0;

//entry
void Do() {
	_rdf.clear();
	_rdf.resize(count);
	rdf = &_rdf[0];

	double v[3];
	double bx[3];
	bx[0] = bbox[1] - bbox[0];
	bx[1] = bbox[3] - bbox[2];
	bx[2] = bbox[5] - bbox[4];
	for (int a = 0; a < pcnt-1; a++) {
		double* m = positions + a*3;
		for (int b = a+1; b < pcnt; b++) {
			double* n = positions + b*3;
			vec(n, m, v);
			periodic(v, bx, v);
			if (fabs(v[0]) < cutoff && fabs(v[1]) < cutoff && fabs(v[2]) < cutoff) {
				double l = len(v);
				auto i = (int)floor(count * l / cutoff);
				if (i >= 0 && i < count) {
					rdf[i] += 2;
				}
			}
		}
	}
	const double dr = cutoff / count;
	const double mul = pcnt * 4 * 3.1415926535 * dr;
	for (int a = 0; a < count; a++) {
		double r = (a+0.5)*dr;
		rdf[a] /= r * r * mul;
	}
}