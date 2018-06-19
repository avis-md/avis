#include <iostream>
#include <fstream>
#include <vector>

VARIN float thres = 0;

VECSZ(fcnt, pcnt, 3)
VARIN float* poss = 0;

VECSZ(fcnt)
VAROUT int* nums = 0;

VECSZ(ccnt)
VAROUT int* cons = 0;

VECVAR int pcnt = 0;
VECVAR int fcnt = 0;
VECVAR int ccnt = 0;

ENTRY void Execute() {
	if (!nums) nums = new int[fcnt];
	
	auto vcons = std::vector<int>();
	
	for (int i = 0; i < fcnt; i++) {
		std::cout << std::endl << i;
		nums[i] = 0;
		float* pos = poss + pcnt * i * 3;
		for (int m = 0; m < pcnt-1; m++) {
			float* pm = pos + m * 3;
			for (int n = m+1; n < pcnt; n++) {
				float* pn = pos + n * 3;
				float dx = pn[0]-pm[0];
				float dy = pn[1]-pm[1];
				float dz = pn[2]-pm[2];
				if ((dx*dx+dy*dy+dz*dz) < thres*thres) {
					nums[i]++;
					vcons.push_back(m);
					vcons.push_back(n);
				}
			}
		}
		std::cout << " " << nums[i];
	}
	ccnt = vcons.size();
	cons = new int[ccnt];
	memcpy(cons, &vcons[0], ccnt * sizeof(int));
}