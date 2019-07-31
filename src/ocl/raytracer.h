// Copyright (C) 2019 Pua Kai
// 
// This file is part of AViS.
// 
// AViS is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// AViS is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with AViS.  If not, see <http://www.gnu.org/licenses/>.

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

	static void Render();
	static void Denoise();

	static void Update();
	static void DrawMenu(float off);

	static uint bgw, bgh;
	static int maxRefl;

	static GLuint resTex;
	static CLWBuffer<float> accum;
	static int samples, maxSamples;
	static int maxSamplesP, maxSamplesR;
	static bool patched, patchedR;
	static int patchSz;
	static bool denoise;
private:
	static std::thread renderThread;
	static bool kill;

	static CLWContext context;
	static CLWProgram program;
	static CLWCommandQueue queue;
	static RR::IntersectionApi* api;

	static REND_STEP rendStep;
	static int rendBounce;

	static std::vector<Vec4> pixels;
	static bool scene_dirty;

	static void SetObjs();
	static void SetSky();
	static void _Refine();
	static CLWBuffer<RR::ray> GeneratePrimaryRays();
	static void ShadeKernel(CLWBuffer<float>& out_buff, const CLWBuffer<RR::Intersection>& isect, CLWBuffer<float>& col_buff, CLWBuffer<RR::ray>& ray_buff, const int smps, const bool isprim);
};
