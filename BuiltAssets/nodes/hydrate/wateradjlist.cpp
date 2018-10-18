#include <iostream>
#include <vector>

//in pcnt lsz
int* adjlist = 0;
//in
int listsz = 0;
//in 1
short* types = 0;

//out pcnt listsz
int* wadjlist = 0;

//var
int pcnt = 0;
//var
int lsz = 0;

std::vector<int> vec;

//entry
void Do() {
	vec.clear();
	vec.resize(pcnt*listsz, -1);
	for (int a = 0; a < pcnt;) {
		if (types[a] == *(short*)"O") {
			int* tar = &vec[a*listsz];
			for (int b = 0; b < 3; b++) {
				for (int c = 0; c < lsz; c++) {
					int cn = adjlist[(a+b)*lsz + c];
					if (cn == -1) break;
					while (types[cn] != *(short*)"O") cn--;
					*(tar++) = cn;
				}
			}
			a+=4;
		}
		else break;
	}
	wadjlist = &vec[0];
}