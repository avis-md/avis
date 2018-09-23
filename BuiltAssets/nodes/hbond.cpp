#include <iostream>
#include <cmath>
#include <vector>
#include <cstring>

//in
double cutoff = 0;
//in
double cutoffMin = 0;

//in fcnt pcnt 3
double* positions = 0;

//in pcnt
short* types = 0;

//var
int fcnt = 2;
//var
int pcnt = 2;

//out fcnt
int* pairCounts = 0;
//out ccnt
int* bonds = 0;

//var
int ccnt = 0;

//progress
double progress = 0;

#define ISA(s, c) (s == *((short*)c))

//entry
void Execute() {
	progress = 0;
	if (!pairCounts) pairCounts = new int[fcnt];
	auto vcons = std::vector<int>();
	
	const double cut2 = cutoff*cutoff;
	const double cut2m = cutoffMin*cutoffMin;
	for (int i = 0; i < fcnt; i++) {
		progress = i * 1.0f / fcnt;
		pairCounts[i] = 0;
		double* pos = positions + pcnt * i * 3;
#pragma omp parallel for
		for (int m = 0; m < pcnt; m++) {
			//progress += 1.0f / (fcnt * pcnt);
			if (!ISA(types[m], "H")) continue;
			double* pm = pos + m * 3;
			for (int n = 0; n < pcnt; n++) {
				if (!ISA(types[n], "O")) continue;
				
				double* pn = pos + n * 3;
				double dx = pn[0]-pm[0];
				double dy = pn[1]-pm[1];
				double dz = pn[2]-pm[2];
				if (fabsf(dx) < cutoff && fabsf(dy) < cutoff && fabsf(dz) < cutoff) {
					double d = (dx*dx+dy*dy+dz*dz);
					if (d < cut2 && d > cut2m) {
#pragma omp critical
						{
							pairCounts[i]++;
							vcons.push_back(m);
							vcons.push_back(n);
						}
					}
				}
			}
		}
	}
	ccnt = vcons.size();
	if (bonds) delete[](bonds);
	bonds = new int[ccnt];
	memcpy(bonds, &vcons[0], ccnt * sizeof(int));
}