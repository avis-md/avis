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

PROGRS float progress = 0;

#define RD(val) strm.read((char*)&val, sizeof(val))
#define WRT(val) ostrm.write((char*)&val, sizeof(val))
ENTRY void Execute() {
	progress = 0;
	if (!nums) nums = new int[fcnt];
	
	auto vcons = std::vector<int>();
	
	std::ifstream strm("D:\\bnd.bin", std::ios::binary);
	if (strm.is_open()) {
		std::cout << "reading bonds" << std::endl;
		int c = 0;
		for (int i = 0; i < fcnt; i++) {
			progress = i * 1.0f / fcnt;
			RD(nums[i]);
			std::cout << i << " " << nums[i] << std::endl;
			auto co = c;
			c += nums[i] * 2;
			if (!!nums[i]){
				vcons.resize(c);
				strm.read((char*)(&vcons[co]), nums[i] * 2 * sizeof(int));
			}
		}
	}
	else {
		std::cout << "dumping bonds" << std::endl;
		std::ofstream ostrm("D:\\bnd.bin", std::ios::binary);
		int co = 0;
		for (int i = 0; i < fcnt; i++) {
			progress = i * 1.0f / fcnt;
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
			WRT(nums[i]);
			if (!!nums[i]) ostrm.write((char*)(&vcons[co]), nums[i]*2*sizeof(int));
			co += nums[i] * 2;
		}
	}
	ccnt = vcons.size();
	if (cons) delete[](cons);
	cons = new int[ccnt];
	memcpy(cons, &vcons[0], ccnt * sizeof(int));
}