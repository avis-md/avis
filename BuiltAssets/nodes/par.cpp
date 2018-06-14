#include <iostream>
#include <cmath>
#include <cstring>

VARIN float wall = 0;

VECSZ(parCnt, 3)
VARIN float* poss = 0;

VECVAR int parCnt = 2;

VECSZ(rdfSz)
VAROUT float* rdf = 0;

VECVAR int rdfSz = 100;

ENTRY void Execute() {
	if (!rdf) rdf = new float[100] {};
	else memset(rdf, 0, 100*sizeof(float));
	
	auto dw = wall / 100;
	float mypos_x = poss[0];
	float mypos_y = poss[1];
	float mypos_z = poss[2];
	for (int c = 3; c < parCnt * 3; c += 3) {
		auto pos = poss + c;
		float dpx = fabs(pos[0] - mypos_x);
		float dpy = fabs(pos[1] - mypos_y);
		float dpz = fabs(pos[2] - mypos_z);
		dpx /= wall;
		dpx = wall * (dpx - roundf(dpx));
		dpy /= wall;
		dpy = wall * (dpy - roundf(dpy));
		dpz /= wall;
		dpz = wall * (dpy - roundf(dpz));
		
		float dst = sqrt(dpx*dpx + dpy*dpy + dpz*dpz);
		if (dst >= wall) continue;
		int loc = (int)roundf(dst * 100.0f / wall);
		rdf[loc] += 1;
	}
	for (auto a = 0; a < 100; a++) {
		rdf[a] /= (4 * 3.14159f * pow((a+0.5f) * dw, 2) * dw);
	}
}