///filters all guests within 9Å

#include <iostream>
#include <vector>
#include <cmath>

//in pcnt 3
double* poss = 0;
//in 1
short* types = 0;

const double cutoff = 0.9;
const double cutoff2 = cutoff*cutoff;

//out gcnt 2
int* pairs = 0;
std::vector<int> _pairs;

//out pcnt
double* gfilter = 0;

//var
int pcnt = 0;
//var
int gcnt = 0;

//entry
void Do() {
	if (gfilter) delete[](gfilter);
	gfilter = new double[pcnt]{};

	_pairs.clear();
	gcnt = 0;
	for (int a = 0; a < pcnt-1; a++) {
		if (types[a] == *(short*)"C") {
			double* pm = poss + a * 3;
			for (int b = a+1; b < pcnt; b++) {
				if (types[b] == *(short*)"C") {
					double* pn = poss + b * 3;
					double dx = pn[0]-pm[0];
					double dy = pn[1]-pm[1];
					double dz = pn[2]-pm[2];

					if (fabs(dx) < cutoff && fabs(dy) < cutoff && fabs(dz) < cutoff) {
						double d = (dx*dx+dy*dy+dz*dz);
						if (d < cutoff2) {
							_pairs.push_back(a);
							_pairs.push_back(b);
							gfilter[a] = gfilter[b] = 1;
							gcnt++;
						}
					}
				}
			}
		}
	}
	pairs = &_pairs[0];
	std::cout << "num of pairs: " << gcnt << std::endl;
}