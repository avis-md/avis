#include <iostream>
#include <vector>

//in pcnt lcnt
int* adjlist = 0;
//in
int n = 0;

//out pcnt
double* isshape = 0;
std::vector<double> _isshape;

//out rcnt n
int* rings = 0;
std::vector<int> _rings;

//var
int pcnt = 0;
//var
int lcnt = 0;
//var
int rcnt = 0;

bool connd(int i, int j) {
	int* li = adjlist + i*lcnt;
	for (int a = 0; a < lcnt; a++) {
		if (li[a] == -1) return false;
		if (li[a] == j) return true;
	}
	return false;
}

std::vector<int> list;

bool checkvert(int dim, int parent) {
	for (int a = 0; a < lcnt; a++) {
		int me = adjlist[parent*lcnt + a]; //for each vertex connected to parent
		if (me == -1) return false;
		if (dim >= 2 && me == list[dim-2]) //ignore the backward vertex
			continue;
		for (int i = 1; i < dim-1; i++) { //for each backward vertex except -1
			if (connd(me, list[i])) continue; //cannot be connected
		}
		if (dim == n-1) { //if is last vertex
			if (connd(me, list[0])) { //connected to first vertex, done!
				list[dim] = me;
				return true;
			}
		}
		else { //if is not last vertex
			if (dim > 1 && connd(me, list[0])) continue; //vertex 2+ cannot be connected to first vertex
			else { //possible vertex, change children
				list[dim] = me;
				if (checkvert(dim+1, me))
					return true; //answer! return now
				//else: no good child, next
			}
		}
	}
	return false;
}

//entry
void Do() {
	rcnt = 0;
	_isshape.clear();
	_isshape.resize(pcnt, 0);
	isshape = &_isshape[0];
	_rings.clear();

	list.resize(n);

	for (int p0 = 0; p0 < pcnt; p0++) { //for each particle (vertex 0)
		if (isshape[p0]>0) continue; //we only want yes or no
		list[0] = p0;
		if (checkvert(1, p0)) {
			for (auto& v : list) {
				isshape[v] = 1;
				_rings.push_back(v);
			}
			rcnt++;
		}
	}
	rings = &_rings[0];
}