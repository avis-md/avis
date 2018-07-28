#pragma once
#include "Engine.h"
#include "anweb.h"

class AnConv {
public:
	static PyObject* PyArr(int nd, char tp);
	static void* FromPy(PyObject* obj, int dim, int* szs);
	static bool ToPy(void* v, PyObject* obj, int dim, int* szs);
};