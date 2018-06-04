#include "anconv.h"

uint AnConv::FromPy_Sz(PyObject* o, int** sz, int dim) {
	**sz = PyList_Size(o);
	if (dim > 0) {
		auto o2 = PyList_GetItem(o, 0);
		return **sz * FromPy_Sz(o2, sz + 1, dim - 1);
	}
	else return **sz;
}

void AnConv::FromPy_Do(PyObject* o, int** sz, int d, float* v) {
	for (Py_ssize_t a = 0; a < **sz; a++) {
		auto o2 = PyList_GetItem(o, a);
		if (d > 0)
			FromPy_Do(o2, sz + 1, d - 1, v);
		else
			*(v++) = (float)PyFloat_AsDouble(o2);
	}
}

float* AnConv::FromPy(PyObject* o, int dim, int** szs) {
	uint sz = FromPy_Sz(o, szs, dim-1);
	float* res = new float[sz];
	FromPy_Do(o, szs, dim-1, res);
	return res;
}

PyObject* AnConv::ToPy(float* v, int dim, int* szs) {
	return nullptr;
}