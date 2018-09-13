#pragma once
#include "Engine.h"
#include "radeon_rays_cl.h"
#include "CLW.h"

#ifdef PLATFORM_WIN
#pragma comment(lib, "RadeonRays.lib")
#pragma comment(lib, "CLW.lib")
#pragma comment(lib, "OpenCL.lib")
#endif

namespace RR = RadeonRays;

class RayTracer {
public:
	static bool Init();
	
	static void Clear();
	static void SetScene();
	static void Render();

	static void DrawMenu();

	static GLuint resTex;

private:
	static CLWContext context;
	static CLWProgram program;
	static CLWCommandQueue queue;
	static RR::IntersectionApi* api;
};