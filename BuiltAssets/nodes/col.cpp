#include <iostream>
#include <fstream>
#include <random>

VARIN float wall = 0;
VARIN float str = 0;

VECSZ(tcnt, pcnt, 3)
VARIN float* poss = 0;

VECSZ(tcnt, pcnt)
VAROUT float* cols = 0;

//VECVAR int tcnti = 2;
//VECVAR int pcnti = 2;

VECVAR int tcnt = 2;
VECVAR int pcnt = 2;

float ds2 (float dx, float dy, float dz) {
	dx /= wall;
	dx -= round(dx);
	dx *= wall;
	dy /= wall;
	dy -= round(dy);
	dy *= wall;
	dz /= wall;
	dz -= round(dz);
	dz *= wall;
	return dx*dx + dy*dy + dz*dz;
}

#define RD(val) strm.read((char*)&val, sizeof(val))
#define WRT(val) strm.write((char*)&val, sizeof(val))

ENTRY void Execute() {
	if (!cols) cols = new float[tcnt * pcnt] {};
	
	std::ifstream strm("D://ljp.bin", std::ios::binary);
	if (strm.is_open()) {
		std::cout << "reading lj potentials" << std::endl;
		int t;
		float f;
		RD(t);
		RD(t);
		for (int t = 0; t < tcnt; t++) {
			for (int i = 0; i < pcnt; i++) {
				RD(f);
				//if (i == 217) std::cout << f << std::endl;
				cols[t * pcnt + i] = -f * str;
			}
		}
	}
	else {
		std::ofstream strm("D://ljp.bin", std::ios::binary);
		if (strm.is_open()) {
			std::cout << "dumping lj potentials" << std::endl;
			WRT(tcnt);
			WRT(pcnt);
			for (int t = 0; t < tcnt; t++) {
				std::cout << t << std::endl;
				float* p0 = poss + t * pcnt * 3;
				for (int i = 0; i < pcnt; i++) {
					float lj = 0;
					float xi = p0[i * 3];
					float yi = p0[i * 3 + 1];
					float zi = p0[i * 3 + 2];
					for (int j = 0; j < pcnt; j++) {
						if (i != j) {
							float xj = p0[j * 3];
							float yj = p0[j * 3 + 1];
							float zj = p0[j * 3 + 2];
							float dp2 = ds2(xj-xi, yj-yi, zj-zi) * 100;
							lj += 4 * (1 / pow(dp2, 6) - 1 / pow(dp2, 3));
						}
					}
					lj /= pcnt;
					WRT(lj);
					cols[t * pcnt + i] = -lj * str;
				}
			}
			strm.close();
		}
	}
}