#pragma once
#include "Engine.h"
#include "web/anweb.h"
#include <Python.h>

class PyArr {
public:
	static int Init();
	
	static void* FromPy(PyObject* obj, int dim, int stride, int* szs, int& tsz);
	static PyObject* ToPy(void* data, int dim, char tp, int* szs);
	
	static std::string GetTypeName(int type);
};