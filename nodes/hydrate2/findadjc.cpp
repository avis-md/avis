///find the 2 adjacent guest(C) atoms
///guests are shaded 0.5, and the rings shaded 1

#include <iostream>
#include <vector>
#include <cmath>

//in pcnt 3
double* poss = 0;
//in 1
short* types = 0;
//in rcnt rn
int* rings = 0;
//in
double maxdist = 0;
///in range 0 1
//double maxangle = 0;

std::vector<double> ringc;
std::vector<int> guests;
int gcnt = 0;

//out pcnt
double* vals = 0;
std::vector<double> _vals;

//var
int pcnt = 0;
//var
int rcnt = 0;
//var
int rn = 0;

void vec(double* to, double* frm, double* v) {
	v[0] = to[0] - frm[0];
	v[1] = to[1] - frm[1];
	v[2] = to[2] - frm[2];
}

double dist2(double* a, double* b) {
	double tmp[3];
	vec(b, a, tmp);
	return tmp[0]*tmp[0] + tmp[1]*tmp[1] + tmp[2]*tmp[2];
}

double dist(double* a, double* b) {
	return sqrt(dist2(a, b));
}

double dot(double* a, double* b) {
	return a[0]*b[0] + a[1]*b[1] + a[2]*b[2];
}

void getringcs() {
	ringc.clear();
	ringc.resize(rcnt*3, 0);
	for (int a = 0; a < rcnt; a++) {
		double* cp = &ringc[a*3];
		for (int b = 0; b < rn; b++) {
			double* ps = poss + rings[a*rn + b]*3;
			cp[0] += ps[0];
			cp[1] += ps[1];
			cp[2] += ps[2];
		}
		cp[0] /= rn;
		cp[1] /= rn;
		cp[2] /= rn;
	}
}

void getguests() {
	guests.clear();
	guests.reserve(pcnt/2);
	for (int a = 0; a < pcnt; a++) {
		if (types[a] == *(short*)"C") {
			guests.push_back(a);
		}
	}
	gcnt = guests.size();
	std::cout << "guest count: " << gcnt << std::endl;
}

bool insideloop(int ri, int gi) {
	double* gp = poss + gi*3;
	double* cp = &ringc[ri*3];
	for (int a = 0; a < rn; a++) {
		double* vp = poss + rings[ri*rn + a];
		double t1[3], t2[3];
		vec(cp, vp, t1); vec(gp, vp, t1);
		if (dot(t1, t2) < 0) return false;
	}
	return true;
}

//entry
void Do() {
	_vals.clear();
	_vals.resize(pcnt, 0);
	vals = &_vals[0];
	
	getringcs();
	getguests();
	double md2 = maxdist*maxdist;
	for (int a = 0; a < rcnt; a++) { //for each ring
		std::vector<int> gs;
		for (int b = 0; b < gcnt; b++) { //for each guest
			auto g = guests[b];
			if (dist2(&ringc[a*3], &poss[g*3]) > md2) //near enough?
				continue; //too far, nope
			if (!insideloop(a, b)) //inside loop?
				continue; //outside, nope
			
			gs.push_back(g); //potential guest, insert
		}
		std::cout << "c: " << gs.size() << std::endl;
		if (gs.size() > 1) { //at least 2 guests?
			//double d1[3], d2[3];
			//vec(poss + gs[0]*3, &ringc[a*3], d1);
			//vec(poss + gs[1]*3, &ringc[a*3], d2);
			//if (dot(d1, d2) < 0) { //different side?
				vals[gs[0]] = vals[gs[1]] = 0.5f;
				for (int r = 0; r < rn; r++) {
					vals[rings[a*rn + r]] = 1;
				}
			//}
		}
	}
}