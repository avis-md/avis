#pragma once
#include "Engine.h"
#include "anweb.h"

class AnConv {
public:
	static int Init();
	
	static PyObject* PyArr(char tp, int nd, int* szs, void* data);
	static void* FromPy(PyObject* obj, int dim, int stride, int** szs, int& tsz);
	static bool ToPy(void* v, PyObject* obj, int dim, int* szs);
	
	static std::string GetTypeName(int type);
};