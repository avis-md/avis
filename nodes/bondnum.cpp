#include <iostream>
#include <cmath>
#include <vector>
#include <cstring>

//in
double cutoff = 0;
//in pcnt 3
double* positions = 0;

//out pcnt
double* bondnum = 0;

//var
int pcnt = 2;

//entry
void Execute() {
	if (bondnum) delete[] bondnum;
	bondnum = new double[pcnt]{};
	
	const double cut2 = cutoff*cutoff;
	for (int m = 0; m < pcnt-1; m++) {
		double* pm = positions + m * 3;
		for (int n = m+1; n < pcnt; n++) {
			double* pn = positions + n * 3;
			double dx = pn[0]-pm[0];
			double dy = pn[1]-pm[1];
			double dz = pn[2]-pm[2];
			if (fabs(dx) < cutoff && fabs(dy) < cutoff && fabs(dz) < cutoff) {
				double d = (dx*dx+dy*dy+dz*dz);
				if (d < cut2) {
					bondnum[m] += 0.1f;
					bondnum[n] += 0.1f;
				}
			}
		}
	}
}