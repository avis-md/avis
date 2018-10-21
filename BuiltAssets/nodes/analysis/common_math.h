#pragma once
#include <cmath>

void vec(double* to, double* frm, double* v) {
	v[0] = to[0] - frm[0];
	v[1] = to[1] - frm[1];
	v[2] = to[2] - frm[2];
}

double len2(double* a) {
	return a[0]*a[0] + a[1]*a[1] + a[2]*a[2];
}

double len(double* a) {
	return sqrt(len2(a));
}

double dist2(double* a, double* b) {
	double tmp[3];
	vec(b, a, tmp);
	return len2(tmp);
}

double dist(double* a, double* b) {
	return sqrt(dist2(a, b));
}

void norm(double* v, double* r) {
	double d = len(v);
	r[0] = v[0] / d;
	r[1] = v[1] / d;
	r[2] = v[2] / d;
}

double dot(double* a, double* b) {
	return a[0]*b[0] + a[1]*b[1] + a[2]*b[2];
}

double periodic(double v, double len) {
	return v - len * round(v / len);
}

void periodic(double* v, double* box, double* vn) {
	vn[0] = periodic(v[0], box[0]);
	vn[1] = periodic(v[1], box[1]);
	vn[2] = periodic(v[2], box[2]);
}