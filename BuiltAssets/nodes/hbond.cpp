#include <iostream>
#include <cmath>
#include <vector>
#include <cstring>

VARIN float cutoff = 0;
VARIN float cutoffMin = 0;

VECSZ(fcnt, pcnt, 3)
VARIN float* positions = 0;

VECSZ(pcnt)
VARIN short* types = 0;

VECVAR int fcnt = 2;
VECVAR int pcnt = 2;

VECSZ(fcnt)
VAROUT int* pairCounts = 0;
VECSZ(ccnt)
VAROUT int* bonds = 0;

VECVAR int ccnt = 0;

PROGRS float progress = 0;

#define ISA(s, c) (s == *((short*)c))

ENTRY Execute() {
	progress = 0;
	if (!pairCounts) pairCounts = new int[fcnt];
	auto vcons = std::vector<int>();
	
	const float cut2 = cutoff*cutoff;
	const float cut2m = cutoffMin*cutoffMin;
	for (int i = 0; i < fcnt; i++) {
		progress = i * 1.0f / fcnt;
		pairCounts[i] = 0;
		float* pos = positions + pcnt * i * 3;
#pragma omp parallel for
		for (int m = 0; m < pcnt; m++) {
			//progress += 1.0f / (fcnt * pcnt);
			if (!ISA(types[m], "H")) continue;
			float* pm = pos + m * 3;
			for (int n = 0; n < pcnt; n++) {
				if (!ISA(types[n], "O")) continue;
				
				float* pn = pos + n * 3;
				float dx = pn[0]-pm[0];
				float dy = pn[1]-pm[1];
				float dz = pn[2]-pm[2];
				if (fabsf(dx) < cutoff && fabsf(dy) < cutoff && fabsf(dz) < cutoff) {
					float d = (dx*dx+dy*dy+dz*dz);
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