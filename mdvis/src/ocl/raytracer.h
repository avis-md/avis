#pragma once
#include "Engine.h"
#define CL_USE_DEPRECATED_OPENCL_1_2_APIS
#include <CL/cl.h>
#ifdef PLATFORM_WIN
#pragma comment(lib, "OpenCL.lib")
#endif

class RayTracer {
public:
	static struct info_st {
		cl_int w;
		cl_int h;
		cl_float IP[16];
		cl_int rand;
	} info;

	static bool Init();
	
	static bool live;

	static void SetTex(uint w, uint h);
	static void Render();

	static GLuint resTex;
	
	static cl_context _ctx;
	static cl_command_queue _que;
	static cl_kernel _kernel;
};