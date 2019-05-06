#pragma once
#include "Engine.h"
#include "radeon_rays_cl.h"
#include "CLW.h"
#include <atomic>

#ifdef PLATFORM_WIN
#pragma comment(lib, "RadeonRays.lib")
#pragma comment(lib, "CLW.lib")
#pragma comment(lib, "OpenCL.lib")
#endif

namespace RR = RadeonRays;

namespace RadeonRays {
	class MatFunc {
	public:
		static RR::matrix Glm2RR(const glm::mat4& mat);

		static RR::matrix Translate(const Vec3& v);
		static RR::matrix Scale(const Vec3& v);
	};
}

class RayTracer {
	enum class REND_STEP {
		WAIT_SCENE,
		IDLE,
		WAIT_PRIMARY,
		WAIT_INTERSECT,
		WAIT_BOUNCE
	};
public:
	static bool Init();
	static void Cleanup();
	
	static void Clear();
	static void SetScene();
	static void UnsetScene();
	static void _SetScene();

	static void Refine();
	static void RefineAsync();
	static void Render();
	static void Denoise();

	static void Update();
	static void DrawMenu();

	static uint bgw, bgh;
	static int maxRefl;

	static GLuint resTex;
	static CLWBuffer<float> accum;
	static int samples, maxSamples;
private:
	static std::thread renderThread;
	static bool kill;

	static CLWContext context;
	static CLWProgram program;
	static CLWCommandQueue queue;
	static RR::IntersectionApi* api;

	static REND_STEP rendStep;
	static int rendBounce;
	static RR::Event* rendEvent;
	static CLWEvent rendEvent2;

	static std::vector<Vec4> pixels;
	static bool scene_dirty;

	static void SetObjs();
	static void SetSky();
	static void _Refine();
	static CLWBuffer<RR::ray> GeneratePrimaryRays();
	static void ShadeKernel(CLWBuffer<float> out_buff, const CLWBuffer<RR::Intersection>& isect, CLWBuffer<float>& col_buff, CLWBuffer<RR::ray>& ray_buff, const int smps, const bool isprim);
};
