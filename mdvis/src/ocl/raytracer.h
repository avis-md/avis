#pragma once
#include "Engine.h"
#include "utils/BVH.h"
#define CL_USE_DEPRECATED_OPENCL_1_2_APIS
#ifdef PLATFORM_OSX
#include <OpenCL/cl.h>
#else
#include <CL/cl.h>
#endif
#ifdef PLATFORM_WIN
#pragma comment(lib, "OpenCL.lib")
#endif

class RayTracer {
public:
	static bool Init();
	
	static void Clear();
	static void SetScene();
	static void Render();

	static void DrawMenu();

	static GLuint resTex;

};