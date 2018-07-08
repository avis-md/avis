#pragma once
#include "Engine.h"
#include "utils/BVH.h"
#define CL_USE_DEPRECATED_OPENCL_1_2_APIS
#include <CL/cl.h>
#ifdef PLATFORM_WIN
#pragma comment(lib, "OpenCL.lib")
#endif

class RayTracer {
public:
	struct mat_st {
		cl_float rough;
		cl_float specular;
		cl_float gloss;
		cl_float metallic;
	};
	static struct info_st {
		cl_int w;
		cl_int h;
		cl_float IP[16];
		cl_int rand;
		cl_float str;
		cl_int bg_w;
		cl_int bg_h;
		mat_st mat;
	} info;

	static bool Init();
	
	static bool live;
	static BVH::Node* bvh;
	static uint bvhSz;

	static void SetScene();
	static void SetTex(uint w, uint h);
	static void Render();
	static void Clear();

	static void Draw();

	static GLuint resTex;
	
	static cl_context _ctx;
	static cl_command_queue _que;
	static cl_kernel _kernel;
};