#include "raytracer.h"
#include "hdr.h"
#include "kernel.h"
#include "vis/pargraphics.h"

RadeonRays::matrix RadeonRays::MatFunc::Glm2RR(const glm::mat4& mat) {
	auto m = glm::transpose(mat);
	return *(RadeonRays::matrix*)&m;
}

RR::matrix RR::MatFunc::Translate(const Vec3& v) {
	return RR::matrix(1, 0, 0, v.x, 0, 1, 0, v.y, 0, 0, 1, v.z, 0, 0, 0, 1);
}

RR::matrix RR::MatFunc::Scale(const Vec3& v) {
	return RR::matrix(v.x, 0, 0, 0, 0, v.y, 0, 0, 0, 0, v.z, 0, 0, 0, 0, 1);
}

int RayTracer::maxRefl = 3;

GLuint RayTracer::resTex = 0;
CLWBuffer<float> RayTracer::accum;
int RayTracer::samples = 0;

CLWContext RayTracer::context;
CLWProgram RayTracer::program;
CLWCommandQueue RayTracer::queue;
RR::IntersectionApi* RayTracer::api;

#define FAIL(txt) Debug::Warning("RayTracer", txt);\
return false
bool RayTracer::Init() {
	std::vector<CLWPlatform> platforms;
	try {
		CLWPlatform::CreateAllPlatforms(platforms);
	}
	catch (CLWException& e) {
		Debug::Warning("RayTracer", e.what());
	}

	if (platforms.size() == 0)
	{
		FAIL("No OpenCL platforms installed.");
	}

	for (int i = 0; i < platforms.size(); ++i) {
		for (int d = 0; d < (int)platforms[i].GetDeviceCount(); ++d) {
			if (platforms[i].GetDevice(d).GetType() == CL_DEVICE_TYPE_GPU) {
				try {
					context = CLWContext::Create(platforms[i].GetDevice(d));
					break;
				}
				catch (CLWException& e) {
					Debug::Warning("RayTracer", e.what());
				}
			}
		}
		if (context) break;
	}
	if (!context) {
		FAIL("Cannot find a GPU context!");
	}
	const char* kBuildopts(" -cl-mad-enable -cl-fast-relaxed-math -cl-std=CL1.2 -I.");

	try {
		auto kernel = IO::GetText(IO::path + "res/kernel.cl");
		program = CLWProgram::CreateFromSource(kernel.c_str(), kernel.size(), kBuildopts, context);
	}
	catch (CLWException& e) {
		FAIL(e.what());
	}

	auto id = context.GetDevice(0).GetID();
	queue = context.GetCommandQueue(0);
	
	try {
		api = RR::CreateFromOpenClContext(context, id, queue);
	}
	catch (std::runtime_error& err) {
		 Debug::Warning("RayTracer::InitRR", err.what());
		 return false;
	}

	return true;
}

std::vector<shape_t> g_objshapes;
std::vector<material_t> g_objmaterials;

CLWBuffer<float> bg_buf;
CLWBuffer<float> g_positions;
CLWBuffer<float> g_normals;
CLWBuffer<Vec4> g_colors;
CLWBuffer<int> g_indices;
CLWBuffer<int> g_indent;
CLWBuffer<RR::matrix> g_matrices;

void RayTracer::Clear() {
	accum = CLWBuffer<float>();
	bg_buf = CLWBuffer<float>();
	g_positions = CLWBuffer<float>();
	g_normals = CLWBuffer<float>();
	g_colors = CLWBuffer<Vec4>();
	g_indices = CLWBuffer<int>();
	g_indent = CLWBuffer<int>();
	g_matrices = CLWBuffer<RR::matrix>();
	resTex = 0;
}

void RayTracer::SetScene() {
	if (!api) {
		Debug::Message("System", "Initializing RayTracer");
		Init();
	}
	const int wh = Display::width * Display::height;
	accum = CLWBuffer<float>::Create(context, CL_MEM_READ_WRITE, 3 * wh);

	SetObjs();

	glGenTextures(1, &resTex);
	glBindTexture(GL_TEXTURE_2D, resTex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, Display::width, Display::height, 0, GL_RGB, GL_UNSIGNED_BYTE, 0);
	SetTexParams<>();
	glBindTexture(GL_TEXTURE_2D, 0);
}

void RayTracer::Refine() {
	const int wh = Display::width * Display::height;

	if (Scene::dirty) {
		samples = 0;
	}

	CLWBuffer<RR::ray> ray_buffer_cl = GeneratePrimaryRays();
	RR::Buffer* ray_buffer = CreateFromOpenClBuffer(api, ray_buffer_cl);
	// Intersection data
	CLWBuffer<RR::Intersection> isect_buffer_cl = CLWBuffer<RR::Intersection>::Create(context, CL_MEM_READ_WRITE, wh);
	RR::Buffer* isect_buffer = CreateFromOpenClBuffer(api, isect_buffer_cl);

	// Intersection
	api->QueryIntersection(ray_buffer, wh, isect_buffer, nullptr, nullptr);

	auto out_buff = CLWBuffer<byte>::Create(context, CL_MEM_WRITE_ONLY, 4 * wh);
	auto col_buff = CLWBuffer<float>::Create(context, CL_MEM_READ_WRITE, 4 * wh);

	// Shading
	RR::Event* e = nullptr;

	for (int a = 0; a < maxRefl-1; ++a) {
		ShadeKernel(out_buff, isect_buffer_cl, col_buff, ray_buffer_cl, 1 + samples, a==0);
		delete(ray_buffer);
		ray_buffer = CreateFromOpenClBuffer(api, ray_buffer_cl);
		api->QueryIntersection(ray_buffer, wh, isect_buffer, nullptr, &e);
		e->Wait();
	}
	ShadeKernel(out_buff, isect_buffer_cl, col_buff, ray_buffer_cl, ++samples, false);
	delete(ray_buffer);

	void* pixels = clEnqueueMapBuffer(queue, (cl_mem)out_buff, true, CL_MAP_READ, 0, 4 * wh, 0, NULL, NULL, NULL);

	// Update texture data
	glBindTexture(GL_TEXTURE_2D, resTex);
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, Display::width, Display::height, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
	glBindTexture(GL_TEXTURE_2D, 0);

	clEnqueueUnmapMemObject(queue, (cl_mem)out_buff, pixels, 0, NULL, NULL);

	delete(isect_buffer);

	std::cout << samples << std::endl;
}

void RayTracer::Render() {
	
}

void RayTracer::DrawMenu() {

}

CLWBuffer<RR::ray> RayTracer::GeneratePrimaryRays() {
	struct Cam
	{
		Mat4x4 ip = glm::transpose(glm::inverse(ParGraphics::lastMVP));
		Vec2 zcap = Vec2(1, 1000);
	} cam = Cam();
	CLWBuffer<Cam> cam_buf = CLWBuffer<Cam>::Create(context, CL_MEM_READ_ONLY, 1, &cam);

	//run kernel
	CLWBuffer<RR::ray> ray_buf = CLWBuffer<RR::ray>::Create(context, CL_MEM_READ_WRITE, Display::width * Display::height);
	CLWKernel kernel = program.GetKernel("GenerateCameraRays");
	kernel.SetArg(0, ray_buf);
	kernel.SetArg(1, cam_buf);
	kernel.SetArg(2, Display::width);
	kernel.SetArg(3, Display::height);
	// Run generation kernel
	size_t gs[] = { static_cast<size_t>((Display::width + 7) / 8 * 8), static_cast<size_t>((Display::height + 7) / 8 * 8) };
	size_t ls[] = { 8, 8 };
	context.Launch2D(0, gs, ls, kernel);
	context.Flush(0);

	return ray_buf;
}

#include "asset/tetrahedron.h"

void RayTracer::SetObjs() {
	Tetrahedron tet = Tetrahedron();
	for (int a = 0; a < 4; a++)
		tet.Subdivide();
	tet.ToSphere(0.15f);

	g_objshapes.push_back(shape_t());
	auto& m = g_objshapes.back().mesh;
	auto& v = m.positions;
	v.resize(tet.verts.size() * 3);
	memcpy(&v[0], &tet.verts[0], tet.verts.size() * sizeof(Vec3));
	m.indices = tet.tris;
	m.material_ids.resize(tet.tris.size()/3, 0);
	m.normals.resize(tet.norms.size() * 3);
	memcpy(&m.normals[0], &tet.norms[0], tet.norms.size() * sizeof(Vec3));
	const uint mp = Particles::particleSz;

	std::vector<double>* attr = 0;
	if (ParGraphics::useGradCol) {
		attr = &Particles::attrs[ParGraphics::gradColParam]->Get(Particles::anim.currentFrame);
	}

	std::vector<float> verts;
	std::vector<float> normals;
	std::vector<int> inds;
	std::vector<Vec4> colors;
	std::vector<int> indents(mp, 0);
	std::vector<RR::matrix> matrices;

	const mesh_t& mesh = g_objshapes[0].mesh;
	verts.insert(verts.end(), mesh.positions.begin(), mesh.positions.end());
	normals.insert(normals.end(), mesh.normals.begin(), mesh.normals.end());
	inds.insert(inds.end(), mesh.indices.begin(), mesh.indices.end());

	if (mesh.positions.size() / 3 < mesh.indices.size())
	{
		int count = mesh.indices.size() * 3 - mesh.positions.size();
		auto sz = verts.size();
		verts.resize(sz + count, 0.f);
		normals.resize(sz + count, 0.f);
	}

	colors.reserve(mp);
	matrices.reserve(mp);
	for (int id = 0; id < mp; ++id) {
		if (!ParGraphics::useGradCol || !attr->size()) {
			colors.push_back(Particles::_colorPallete[Particles::colors[id]]);
		}
		else {
			auto col = Color::HueBaseCol(Clamp((1 - (float)(*attr)[id]), 0.f, 1.f) * 0.6667f);
			colors.push_back(col);
		}

		matrices.push_back(RR::MatFunc::Translate(Particles::poss[id]));
	}

	g_positions = CLWBuffer<float>::Create(context, CL_MEM_READ_ONLY, verts.size(), verts.data());
	g_normals = CLWBuffer<float>::Create(context, CL_MEM_READ_ONLY, normals.size(), normals.data());
	g_indices = CLWBuffer<int>::Create(context, CL_MEM_READ_ONLY, inds.size(), inds.data());
	g_colors = CLWBuffer<Vec4>::Create(context, CL_MEM_READ_ONLY, mp, colors.data());
	g_indent = CLWBuffer<int>::Create(context, CL_MEM_READ_ONLY, mp, indents.data());
	g_matrices = CLWBuffer<RR::matrix>::Create(context, CL_MEM_READ_ONLY, mp, matrices.data());

	unsigned int _w, _h;
	auto d = hdr::read_hdr((IO::path + "res/refl.hdr").c_str(), &_w, &_h);
	std::vector<float> dv(_w*_h*3);
	hdr::to_float(d, _w, _h, dv.data());
	bg_buf = CLWBuffer<float>::Create(context, CL_MEM_READ_ONLY, 3 * _w * _h, dv.data());
	delete[](d);

	auto& objshape = g_objshapes[0];
	float* vertdata = objshape.mesh.positions.data();
	int nvert = objshape.mesh.positions.size() / 3;
	int* indices = objshape.mesh.indices.data();
	int nfaces = objshape.mesh.indices.size() / 3;
	RR::Shape* shape0 = api->CreateMesh(vertdata, nvert, 3 * sizeof(float), indices, 0, nullptr, nfaces);
	for (int id = 0; id < mp; ++id) {
		auto shp = (!id) ? shape0 : api->CreateInstance(shape0);
		shp->SetTransform(matrices[id], RR::inverse(matrices[id]));
		shp->SetId(id);
		api->AttachShape(shp);
	}

	api->Commit();
}

void RayTracer::ShadeKernel(CLWBuffer<byte> out_buff, const CLWBuffer<RR::Intersection>& isect, CLWBuffer<float>& col_buff, CLWBuffer<RR::ray>& ray_buff, const int smps, const bool isprim)
{
	//run kernel
	CLWKernel kernel = program.GetKernel("Shading");
	int i = 0;
#define karg(var) kernel.SetArg(i++, var)
	karg(g_positions);
	karg(g_normals);
	karg(g_indices);
	karg(g_colors);
	karg(g_indent);
	karg(g_matrices);
	karg(isect);
	karg(isprim? 1 : 0);
	karg(Display::width);
	karg(Display::height);
	karg(out_buff);
	karg(col_buff);
	karg(ray_buff);
	karg(cl_int(rand() % RAND_MAX));
	karg(accum);
	karg(smps);
	karg(bg_buf);
	karg(2.0f);

	// Run generation kernel
	size_t gs[] = { static_cast<size_t>((Display::width + 7) / 8 * 8), static_cast<size_t>((Display::height + 7) / 8 * 8) };
	size_t ls[] = { 8, 8 };
	context.Launch2D(0, gs, ls, kernel);
	context.Flush(0);
}