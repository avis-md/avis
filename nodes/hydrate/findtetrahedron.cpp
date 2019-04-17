#include <iostream>
#include <vector>
#include <utility>

//in
int pcnt = 0;
//in ecnt 2
int* edges = 0;
//in gcnt 3
int* links = 0;

//out mcnt 4
int* result = 0;
std::vector<int> _result;

//out pcnt
double* mask = 0;

//var
int ecnt = 0;
//var
int gcnt = 0;
//var
int mcnt = 0;

const int vcnt = 3;

#define NARR(tp, nm, sz) if (nm) delete[](nm); nm = new tp[sz]{};

bool share_edge(int g1, int g2, int& rv1, int& rv2) {
	int sv = 0;
	for (int a = 0; a < vcnt; a++) {
		auto v1 = links[g1 * vcnt + a];
		for (int b = 0; b < vcnt; b++) {
			auto v2 = links[g2 * vcnt + b];
			if (v1 == v2) {
				sv++;
				if (sv == 2) {
					rv2 = v1;
					return true;
				}
				else rv1 = v1;
			}
		}
	}
	return false;
}

bool is_tetra(int g1, int g2, int* vs) {
	int v1, v2;
	if (!share_edge(g1, g2, v1, v2)) return false;
	int nv1, nv2;
	for (int a = 0; a < vcnt; a++) {
		auto v = links[g1 * vcnt + a];
		if (v != v1 && v != v2) {
			nv1 = v;
			break;
		}
	}
	for (int a = 0; a < vcnt; a++) {
		auto v = links[g2 * vcnt + a];
		if (v != v1 && v != v2) {
			nv2 = v;
			break;
		}
	}
	for (int a = 0; a < ecnt; a++) {
		auto e = &edges[a * 2];
		if ((e[0] == nv1 && e[1] == nv2)
			|| (e[0] == nv2 && e[1] == nv1)) {
			vs[0] = v1;
			vs[1] = v2;
			vs[2] = nv1;
			vs[3] = nv2;
			return true;
		}
	}
	return false;
}

//entry
void Do() {
    NARR(double, mask, pcnt);
	_result.clear();
    mcnt = 0;

	int rs[4];
	for (int a = 0; a < gcnt - 1; a++) {
		for (int b = a + 1; b < gcnt; b++) {
			if (is_tetra(a, b, rs)) {
                mcnt++;
				for (int c = 0; c < 4; c++)
					_result.push_back(rs[c]);
			}
		}
	}
	if (!mcnt) return;
	result = _result.data();
	for (int f = 0; f < 4 * mcnt; f++) {
		mask[result[f]] = 1;
	}
}