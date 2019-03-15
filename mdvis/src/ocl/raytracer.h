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

namespace RadeonRays {
	class MatFunc {
	public:
		static RadeonRays::matrix Glm2RR(const glm::mat4& mat);
	};
}

class RayTracer {
public:

	static bool Init();
	
	static void Clear();
	static void SetScene();

	static void Refine();
	static void Render();

	static void DrawMenu();

	static int maxRefl;

	static GLuint resTex;
	static CLWBuffer<float> accum;
	static int samples;
private:
	static CLWContext context;
	static CLWProgram program;
	static CLWCommandQueue queue;
	static RR::IntersectionApi* api;

	static CLWBuffer<RR::ray> GeneratePrimaryRays();
	static void SetObjs();
	static void ShadeKernel(CLWBuffer<byte> out_buff, const CLWBuffer<RR::Intersection>& isect, CLWBuffer<float>& col_buff, CLWBuffer<RR::ray>& ray_buff, const int smps, const bool isprim);
};

typedef struct
{
	std::string name;

	float ambient[3];
	float diffuse[3];
	float specular[3];
	float transmittance[3];
	float emission[3];
	float shininess;
	float ior;                // index of refraction
	float dissolve;           // 1 == opaque; 0 == fully transparent
								// illumination model (see http://www.fileformat.info/format/material/)
	int illum;

	std::string ambient_texname;
	std::string diffuse_texname;
	std::string specular_texname;
	std::string normal_texname;
	std::map<std::string, std::string> unknown_parameter;
} material_t;

typedef struct
{
	std::vector<float>          positions;
	std::vector<float>          normals;
	std::vector<float>          texcoords;
	std::vector<int>            indices;
	std::vector<int>            material_ids; // per-mesh material ID
} mesh_t;

typedef struct
{
	std::string  name;
	mesh_t       mesh;
} shape_t;