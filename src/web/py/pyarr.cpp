#include "pyarr.h"
#define NPY_NO_DEPRECATED_API NPY_1_7_API_VERSION
#include <numpy/arrayobject.h>

int PyArr::Init() {
	import_array();
	return 0;
}

void* PyArr::FromPy(PyObject* o, int dim, int stride, int* szs, int& tsz) {
	if (!AnWeb::hasPy) return nullptr;
	auto ao = (PyArrayObject*)o;
	auto nd = PyArray_NDIM(ao);
	if (nd != dim) {
		Debug::Warning("Py2C", "wrong array dim! expected " + std::to_string(dim) + ", got " + std::to_string(nd) + "!");
		return 0;
	}
	auto typ = PyArray_TYPE(ao);
	bool sm = false;
	switch (stride) {
	case 2:
		sm = (typ == NPY_SHORT);
		break;
	case 4:
		sm = (typ == NPY_INT32);
		break;
	case 8:
		sm = (typ == NPY_DOUBLE);
		break;
	default:
		break;
	}
	if (!sm) {
		Debug::Warning("Py2C", "Stride does not match internal type! (Stride=" + std::to_string(stride) + ", type=" + GetTypeName(typ) + ")");
		return 0;
	}
	auto shp = PyArray_SHAPE(ao);
	tsz = 1;
	for (int a = 0; a < nd; a++)
		tsz *= (szs[a] = (int)shp[a]);
	return PyArray_DATA(ao);
}

PyObject* PyArr::ToPy(void* data, int dim, char tp, int* szs) {
	if (!AnWeb::hasPy) return nullptr;
	int tn = NPY_FLOAT64;
	int sz = 8;
	switch (tp) {
	case 's':
		tn = NPY_INT16;
		sz = 2;
		break;
	case 'i':
		tn = NPY_INT32;
		sz = 4;
		break;
	case 'd':
		break;
	default: 
		Debug::Warning("AnConv", "unknown type " + std::string(&tp, 1) + "!");
		return nullptr;
	}
	std::vector<npy_intp> dims(dim);
	for (int a = 0; a < dim; a++) dims[a] = (npy_intp)szs[a];
	auto dd = (double*)data;
	return PyArray_SimpleNewFromData(dim, dims.data(), tn, data);
	
}

std::string PyArr::GetTypeName(int type) {
	//https://docs.scipy.org/doc/numpy-1.13.0/reference/c-api.types-and-structures.html#c.PyArray_Descr
	auto tpn = PyArray_DescrFromType(type);
	typedef struct {
		PyObject_HEAD
		PyTypeObject *typeobj;
		//...
	} _tpn;
	return ((_tpn*)tpn)->typeobj->tp_name;
}