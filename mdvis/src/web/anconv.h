#pragma once
#include "Engine.h"
#include "anweb.h"

class AnConv {
public:
	static float* FromPy(PyObject* obj, int dim, int** szs);
	static PyObject* ToPy(float* f, int dim, int* szs);

private:
	static uint FromPy_Sz(PyObject* obj, int** sz, int d);
	static void FromPy_Do(PyObject* obj, int** sz, int d, float* v);
};