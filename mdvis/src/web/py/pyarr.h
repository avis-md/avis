#pragma once
#include "Engine.h"
#include "web/anweb.h"
#include <Python.h>

class PyArr {
public:
	static int Init();
	
	static void* FromPy(PyObject* obj, int dim, int stride, int* szs, int& tsz);
	static bool ToPy(void* v, PyObject* obj, int dim, int* szs);
	
	static std::string GetTypeName(int type);
};