#include <iostream>

//in
int pcnt = 0;
//in
int fcnt = 0;
//in
int showI = 0;

//in gcnt vcnt
int* links = 0;

//out fcnt
int* result = 0;
std::vector<int> _result;

//out pcnt
double* mask = 0;

//var
int gcnt = 0;
//var
int vcnt = 0;

#define NARR(tp, nm, sz) if (nm) delete[](nm); nm = new tp[sz]{};

bool share_edge(int g1, int g2) {
	int sv = 0;
	for (int a = 0; a < vcnt; a++) {
		auto v1 = links[g1 * vcnt + a];
		for (int b = 0; b < vcnt; b++) {
			auto v2 = links[g2 * vcnt + a];
			if (v1 == v2) {
				sv++;
				if (sv == 2) return true;
			}
		}
	}
	return false;
}

//entry
void Do() {
    NARR(double, mask, pcnt);
    
    for (int a = 0; a < gcnt - 1; a++) { //for each face
        std::vector<int> fcs(1, a); //connected faces
		int n = 0;
		for (int b = a + 1; b < gcnt; b++) { //for each other face
			for (auto& f : fcs) { //for each connected face
				if (!share_edge(f, b)) //is face 2 connected to 
					goto notatt;
			}
			fcs.push_back(b);
			if (++n == fcnt) {
				_result = fcs;
				result = _result.data();
				goto found;
			}
		notatt:
			continue;
		}
    }
	return;
found:
	for (int f = 0; f < fcnt; f++) {
		auto rf = result[f];
		for (int a = 0; a < vcnt; a++) {
			mask[links[rf * vcnt + a]] = 1;
		}
	}
}