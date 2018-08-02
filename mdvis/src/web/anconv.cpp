#include "anconv.h"
#define NPY_NO_DEPRECATED_API NPY_1_7_API_VERSION
#include <numpy/arrayobject.h>

PyObject* AnConv::PyArr(int nd, char tp) {
	if (!AnWeb::hasPy) return nullptr;
	assert(nd <= 10);
	npy_intp zs[10]{};
	int tn = NPY_FLOAT;
	int sz = 4;
	switch (tp) {
	case 'i':
		tn = NPY_INT;
		sz = sizeof(int);
		break;
	case 's':
		tn = NPY_SHORT;
		sz = 2;
		break;
	default: break;
	}
	return PyArray_New(&PyArray_Type, nd, zs, tn, nullptr, nullptr, sz, 0, nullptr);
}

void* AnConv::FromPy(PyObject* o, int dim, int** szs) {
	if (!AnWeb::hasPy) return nullptr;
	auto ao = (PyArrayObject*)o;
	auto nd = PyArray_NDIM(ao);
	if (nd != dim) {
		Debug::Warning("Py2C", "wrong array dim! expected " + std::to_string(dim) + ", got " + std::to_string(nd) + "!");
		return 0;
	}
	auto shp = PyArray_SHAPE(ao);
	for (int a = 0; a < nd; a++)
		*(szs[a]) = shp[a];
	auto tp = PyArray_TYPE(ao);
	return PyArray_DATA(ao);
}

bool AnConv::ToPy(void* v, PyObject* obj, int dim, int* szs) {
	if (!AnWeb::hasPy) return false;
	PyArray_Dims dims;
	dims.ptr = (npy_intp*)szs;
	dims.len = dim;
	PyArray_Resize((PyArrayObject*)obj, &dims, 1, NPY_CORDER);
	return true;
}