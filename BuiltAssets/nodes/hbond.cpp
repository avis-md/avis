#include <iostream>
#include <cmath>
#include <vector>
#include <cstring>

VARIN float thres = 0;

VECSZ(fcnt, pcnt, 3)
VARIN float* poss = 0;

VECSZ(pcnt)
VARIN short* typs = 0;

VECVAR int fcnt = 2;
VECVAR int pcnt = 2;

VECSZ(fcnt)
VAROUT int* bndSzs = 0;
VECSZ(ccnt)
VAROUT int* bonds = 0;

VECVAR int ccnt = 0;

PROGRS float progress = 0;

#define ISA(s, c) (s == *((short*)c))

ENTRY Execute() {
	progress = 0;
	if (!bndSzs) bndSzs = new int[fcnt];
	auto vcons = std::vector<int>();
	
	const float thres2 = thres*thres;
	for (int i = 0; i < fcnt; i++) {
		progress = i * 1.0f / fcnt;
		bndSzs[i] = 0;
		float* pos = poss + pcnt * i * 3;
		for (int m = 0; m < pcnt; m++) {
			progress += 1.0f / (fcnt * pcnt);
			if (!ISA(typs[m], "H")) continue;
			float* pm = pos + m * 3;
			for (int n = 0; n < pcnt; n++) {
				if (!ISA(typs[n], "O")) continue;
				
				float* pn = pos + n * 3;
				float dx = pn[0]-pm[0];
				float dy = pn[1]-pm[1];
				float dz = pn[2]-pm[2];
				if (fabsf(dx) < thres && fabsf(dy) < thres && fabsf(dz) < thres) {
					if ((dx*dx+dy*dy+dz*dz) < thres2) {
						bndSzs[i]++;
						vcons.push_back(m);
						vcons.push_back(n);
					}
				}
			}
		}
	}
	ccnt = vcons.size();
	if (!bonds) bonds = new int[ccnt];
	memcpy(bonds, &vcons[0], ccnt * sizeof(int));
}