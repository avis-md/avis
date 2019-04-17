///filters all water atoms
///within 6Å 45° of guests

#include <iostream>
#include <vector>
#include "vmath.h"

//in pcnt 3
double* poss = 0;
//in 1
short* types = 0;
//in gcnt 2
int* gconn = 0;
//in 1
double* nlinked = 0;
//in
double r1 = 0;
//in
double r2 = 0;

const double cutoff = 0.6;
const double cutoff2 = cutoff*cutoff;
const double mcs = 1.0 / sqrt(2.0);

//out pcnt
double* want = 0;
std::vector<double> _want;
//out pcnt
double* radscl = 0;
std::vector<double> _radscl;

//var
int pcnt = 0;
//var
int gcnt = 0;

//entry
void Do() {
	_want.clear();
	_want.resize(pcnt, -1);
	want = &_want[0];
	std::vector<int> wcnt(gcnt);
	_radscl.clear();
	_radscl.resize(pcnt, 1);
	radscl = &_radscl[0];

	std::vector<std::vector<int>> gids(gcnt);

	double d = 0;
	for (int a = 0; a < pcnt; a++) {
		if (types[a] == *(short*)"O") {
			if (nlinked[a] == 0) continue;
			double* pm = poss + a * 3;
			for (int g = 0; g < gcnt; g++) {
				for (int k = 0; k < 2; k++) {
					int b = gconn[g*2 + k];
					double* pn = poss + b * 3;
					double dx = pn[0]-pm[0];
					double dy = pn[1]-pm[1];
					double dz = pn[2]-pm[2];

					if (fabs(dx) > cutoff || fabs(dy) > cutoff || fabs(dz) > cutoff)
						goto nope;
					d = (dx*dx+dy*dy+dz*dz);
					if (d > cutoff2) {
						goto nope;
					}
				}
				{
					double* g1 = poss + gconn[g*2] * 3;
					double* g2 = poss + gconn[g*2 + 1] * 3;
					double t1[3], t2[3], d1[3], d2[3];
					vec(pm, g1, t1); vec(g2, g1, t2);
					norm(t1, d1); norm(t2, d2);
					if (dot(d1, d2) < mcs) continue;
					vec(pm, g2, t1); vec(g1, g2, t2);
					norm(t1, d1); norm(t2, d2);
					if (dot(d1, d2) < mcs) continue;
					gids[g].push_back(a);
					wcnt[g]++;
				}
			nope:;
			}
		}
		else if (types[a] == *(short*)"C") {
			want[a] = 0;
			radscl[a] = r1;
		}
	}
	for (int g = 0; g < gcnt; g++) {
		if (wcnt[g] >= 5) {
			want[gconn[g*2]] = want[gconn[g*2+1]] = 0.5;
			radscl[gconn[g*2]] = radscl[gconn[g*2+1]] = r2;
			for (auto& a : gids[g]) {
				want[a] = 1;
			}
		}
	}
}