#include "raytracer.h"
#include "hdr.h"
#include "kernel.h"
#include "vis/pargraphics.h"
#include "web/anweb.h"
#include "denoise.h"
#include "unloader.h"
//#include "clext.h"

#define clErrorString(a) std::string(" ???")

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

Int2 patchPos = { 0, 0 };
Int2 patchSize;

uint RayTracer::bgw, RayTracer::bgh;
int RayTracer::maxRefl = 5;

GLuint RayTracer::resTex = 0;
CLWBuffer<float> RayTracer::accum;
int RayTracer::samples = 0, RayTracer::maxSamples = 32;

std::thread RayTracer::renderThread;
bool RayTracer::kill;

CLWContext RayTracer::context;
CLWProgram RayTracer::program;
CLWCommandQueue RayTracer::queue;
RR::IntersectionApi* RayTracer::api;

RayTracer::REND_STEP RayTracer::rendStep = RayTracer::REND_STEP::IDLE;
int RayTracer::rendBounce = 0;
RR::Event* RayTracer::rendEvent;
CLWEvent RayTracer::rendEvent2;

std::vector<Vec4> RayTracer::pixels;
bool RayTracer::scene_dirty = false;

cl_ulong rt_maxMem = 0;

#define FAIL(txt) Debug::Warning("RayTracer", txt);\
return false

#define CLWBuffer_New(tp, sz, vl) CLWBuffer<tp>::Create(context, CL_MEM_READ_ONLY, sz, vl);\
Debug::Message("RayTracer", "Creating buffer of size " + std::to_string(sz * sizeof(tp)));\
if (sz * sizeof(tp) > rt_maxMem) Debug::Warning("RayTracer", \
	"Buffer size is too big! (require " + std::to_string(sz * sizeof(tp)) + ", max is " + std::to_string(rt_maxMem) + ")");

bool RayTracer::Init() {
	std::vector<CLWPlatform> platforms;
	try {
		CLWPlatform::CreateAllPlatforms(platforms);
	}
	catch (CLWException& e) {
		Debug::Warning("RayTracer", e.what() + clErrorString(e.errcode_));
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
					Debug::Warning("RayTracer", e.what() + clErrorString(e.errcode_));
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
		program = CLWProgram::CreateFromSource(ocl::raykernel, ocl::raykernel_sz, kBuildopts, context);
	}
	catch (CLWException& e) {
		FAIL(e.what() + clErrorString(e.errcode_));
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

	api->SetOption("bvh.force2level", 1);
	cl_ulong rt_globMem;
	clGetDeviceInfo(context.GetDevice(0), CL_DEVICE_GLOBAL_MEM_SIZE, sizeof(cl_ulong), &rt_globMem, NULL);
	Debug::Message("RayTracer", "Global memory size is " + std::to_string(rt_globMem / 1000000) + "M");
	clGetDeviceInfo(context.GetDevice(0), CL_DEVICE_MAX_MEM_ALLOC_SIZE, sizeof(cl_ulong), &rt_maxMem, NULL);
	Debug::Message("RayTracer", "Max alloc size is " + std::to_string(rt_maxMem / 1000000) + "M");
	
	Unloader::Reg(&Cleanup);
	return true;
}

void RayTracer::Cleanup() {
	UnsetScene();
}

CLWBuffer<float> bg_buf;
CLWBuffer<float> g_positions;
CLWBuffer<float> g_normals;
CLWBuffer<Vec4> g_colors;
CLWBuffer<int> g_indices;
CLWBuffer<int> g_indent;
CLWBuffer<RR::matrix> g_matrices;

CLWBuffer<float> out_buff;
CLWBuffer<float> col_buff;

CLWBuffer<RR::ray> ray_buffer_cl;
CLWBuffer<RR::Intersection> isect_buffer_cl;

RR::Buffer* ray_buffer, *isect_buffer;

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
	pixels.resize(Display::width * Display::height * 4);
	glGenTextures(1, &resTex);
	glBindTexture(GL_TEXTURE_2D, resTex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, Display::width, Display::height, 0, GL_RGBA, GL_FLOAT, pixels.data());
	SetTexParams<>();
	glBindTexture(GL_TEXTURE_2D, 0);

	rendStep = REND_STEP::WAIT_SCENE;
	
	renderThread = std::thread(_Refine);
}

void RayTracer::UnsetScene() {
	kill = true;
	if (renderThread.joinable()) {
		renderThread.join();
	}
	Clear();
}

void RayTracer::_SetScene() {}

void RayTracer::Refine() {
	const int wh = patchSize.x * patchSize.y;//Display::width * Display::height;
	const int dwh = Display::width * Display::height;

	if (Scene::dirty) {
		samples = 0;
	}
	else if (samples >= maxSamples) {
		if (samples == maxSamples) {
			Denoise();
			samples++;
			VisSystem::SetMsg("RayTracing Complete.");
		}
		return;
	}
	VisSystem::SetMsg("Tracing sample " + std::to_string(samples));
	ray_buffer_cl = GeneratePrimaryRays();
	rendEvent2.Wait();
	isect_buffer_cl = CLWBuffer<RR::Intersection>::Create(context, CL_MEM_READ_WRITE, wh);
	isect_buffer = CreateFromOpenClBuffer(api, isect_buffer_cl);
	for (rendBounce = 0; rendBounce < maxRefl; rendBounce++) {
		ray_buffer = CreateFromOpenClBuffer(api, ray_buffer_cl);
		api->QueryIntersection(ray_buffer, wh, isect_buffer, nullptr, &rendEvent);
		rendEvent->Wait();
		if (!rendBounce) {
			out_buff = CLWBuffer<float>::Create(context, CL_MEM_WRITE_ONLY, 4 * wh);
			col_buff = CLWBuffer<float>::Create(context, CL_MEM_READ_WRITE, 4 * wh);
		}
		ShadeKernel(out_buff, isect_buffer_cl, col_buff, ray_buffer_cl, std::max(samples, 1), !rendBounce);
		rendEvent2.Wait();
		delete(ray_buffer);
	}

	++samples;
	void* pixels = clEnqueueMapBuffer(queue, (cl_mem)out_buff, true, CL_MAP_READ, 0, 4 * wh * sizeof(float), 0, NULL, NULL, NULL);
	// Update texture data
	glBindTexture(GL_TEXTURE_2D, resTex);
	glTexSubImage2D(GL_TEXTURE_2D, 0, patchPos.x, patchPos.y, patchSize.x, patchSize.y, GL_RGBA, GL_FLOAT, pixels);
	glBindTexture(GL_TEXTURE_2D, 0);

	clEnqueueUnmapMemObject(queue, (cl_mem)out_buff, pixels, 0, NULL, NULL);
	delete(isect_buffer);
	if (samples == maxSamples) VisSystem::SetMsg("Denoising...");
}

void RayTracer::RefineAsync() {
	const int wh = patchSize.x * patchSize.y;//Display::width * Display::height;
	const int dwh = Display::width * Display::height;

	if (Scene::dirty) {
		samples = 0;
	}

	switch (rendStep) {
		case REND_STEP::WAIT_SCENE: {
			return;
		}
		case REND_STEP::IDLE: {
			if (samples >= maxSamples) {
				if (samples == maxSamples) {
					Denoise();
					samples++;
					VisSystem::SetMsg("RayTracing Complete.");
				}
				return;
			}
			VisSystem::SetMsg("Tracing sample " + std::to_string(samples));
			rendBounce = 0;
			ray_buffer_cl = GeneratePrimaryRays();
			rendStep = REND_STEP::WAIT_PRIMARY;
			break;
		}
		case REND_STEP::WAIT_PRIMARY: {
			if (rendEvent2.GetCommandExecutionStatus() != CL_COMPLETE) return;
			ray_buffer = CreateFromOpenClBuffer(api, ray_buffer_cl);
			// Intersection data
			isect_buffer_cl = CLWBuffer<RR::Intersection>::Create(context, CL_MEM_READ_WRITE, wh);
			isect_buffer = CreateFromOpenClBuffer(api, isect_buffer_cl);

			// Intersection
			api->QueryIntersection(ray_buffer, wh, isect_buffer, nullptr, &rendEvent);
			context.Flush(0);
			rendStep = REND_STEP::WAIT_INTERSECT;
			break;
		}
		case REND_STEP::WAIT_INTERSECT: {
			//if (!rendEvent->Complete()) return;
			//delete(rendEvent);
			if (!rendBounce) {
				out_buff = CLWBuffer<float>::Create(context, CL_MEM_WRITE_ONLY, 4 * wh);
				col_buff = CLWBuffer<float>::Create(context, CL_MEM_READ_WRITE, 4 * wh);
			}
			ShadeKernel(out_buff, isect_buffer_cl, col_buff, ray_buffer_cl, std::max(samples, 1), !rendBounce);
			rendStep = REND_STEP::WAIT_BOUNCE;
			break;
		}
		case REND_STEP::WAIT_BOUNCE: {
			if (rendEvent2.GetCommandExecutionStatus() != CL_COMPLETE) return;
			delete(ray_buffer);
			if (++rendBounce == maxRefl) {
				++samples;
				void* pixels = clEnqueueMapBuffer(queue, (cl_mem)out_buff, true, CL_MAP_READ, 0, 4 * wh * sizeof(float), 0, NULL, NULL, NULL);
				// Update texture data
				glBindTexture(GL_TEXTURE_2D, resTex);
				glTexSubImage2D(GL_TEXTURE_2D, 0, patchPos.x, patchPos.y, patchSize.x, patchSize.y, GL_RGBA, GL_FLOAT, pixels);
				glBindTexture(GL_TEXTURE_2D, 0);

				clEnqueueUnmapMemObject(queue, (cl_mem)out_buff, pixels, 0, NULL, NULL);
				delete(isect_buffer);
				if (samples == maxSamples) VisSystem::SetMsg("Denoising...");
				rendStep = REND_STEP::IDLE;
			}
			else {
				ray_buffer = CreateFromOpenClBuffer(api, ray_buffer_cl);
				api->QueryIntersection(ray_buffer, wh, isect_buffer, nullptr, &rendEvent);
				rendStep = REND_STEP::WAIT_INTERSECT;
			}
			break;
		}
	}
}

void RayTracer::Render() {
	
}

void RayTracer::Denoise() {
	const int wh = Display::width * Display::height;
	std::vector<float> src(3 * wh), dst(3 * wh);
	std::vector<float> out(4 * wh);

	auto pixels = (float*)clEnqueueMapBuffer(queue, (cl_mem)out_buff, true, CL_MAP_READ, 0, 4 * wh * sizeof(float), 0, NULL, NULL, NULL);
	for (int a = 0; a < wh; a++) {
		std::memcpy(&src[a * 3], pixels + a * 4, sizeof(Vec3));
		out[a * 4 + 3] = pixels[a * 4 + 3];
	}
	clEnqueueUnmapMemObject(queue, (cl_mem)out_buff, pixels, 0, NULL, NULL);
	
	Denoise::Apply(src.data(), Display::width, Display::height, dst.data());

	for (int a = 0; a < wh; a++) {
		std::memcpy(&out[a * 4], &dst[a * 3], sizeof(Vec3));
	}

	glBindTexture(GL_TEXTURE_2D, resTex);
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, Display::width, Display::height, GL_RGBA, GL_FLOAT, out.data());
	glBindTexture(GL_TEXTURE_2D, 0);
}

void RayTracer::Update() {
	if (Scene::dirty) {
		scene_dirty = true;
		return;
	}

	static int _samples = -1;
	if (samples > 0 && samples != _samples) {
		_samples = samples;
		glBindTexture(GL_TEXTURE_2D, resTex);
		glTexSubImage2D(GL_TEXTURE_2D, 0, patchPos.x, patchPos.y, patchSize.x, patchSize.y, GL_RGBA, GL_FLOAT, pixels.data());
		glBindTexture(GL_TEXTURE_2D, 0);
	}
}

void RayTracer::DrawMenu() {

}

#include "asset/tetrahedron.h"
#include "asset/tube.h"

void RayTracer::SetObjs() {
	VisSystem::SetMsg("Loading Meshes");
	uint mp = 0;
	size_t mp2 = 0;
	for (auto& dl : ParGraphics::drawLists) {
		mp += dl.second.first;
	}
	for (auto& dl : ParGraphics::drawListsB) {
		mp2 += dl.second.first;
	}
	mp2 *= 2;

	Tetrahedron tet = Tetrahedron();
	for (int a = 0; a < 4; a++)
		tet.Subdivide();
	tet.ToSphere(1.f);

	Tube tub = Tube(16, 0.015f);

	std::vector<_Mesh*> shapes = { &tet, &tub };
	std::vector<int> _indents;

	size_t s2s = 0;
	_Mesh m;
	std::vector<_Mesh> shapes2;
	for (auto& n : AnWeb::nodes) {
		n->RayTraceMesh(m);
		if (m.triCount > 0) {
			shapes2.push_back(m);
			shapes.push_back(&shapes2.back());
			m = _Mesh();
			s2s++;
		}
	}

	size_t mpt = mp + mp2 + s2s;

	std::vector<double>* attr = 0;
	if (ParGraphics::useGradCol) {
		attr = &Particles::attrs[ParGraphics::gradColParam]->Get(Particles::anim.currentFrame);
	}

	std::vector<float> verts;
	std::vector<float> normals;
	std::vector<int> inds;
	std::vector<Vec4> colors;
	std::vector<int> indents;
	std::vector<RR::matrix> matrices;
	std::vector<RR::matrix> imatrices;

	int vsz = 0;
	for (auto shp : shapes) {
		_indents.push_back(vsz);
		verts.resize(vsz + shp->vertCount * 3);
		std::memcpy(&verts[vsz], shp->vertices.data(), shp->vertCount * sizeof(Vec3));
		normals.resize(vsz + shp->vertCount * 3);
		std::memcpy(&normals[vsz], shp->normals.data(), shp->vertCount * sizeof(Vec3));
		inds.insert(inds.end(), shp->triangles.begin(), shp->triangles.end());

		vsz = std::max(verts.size(), inds.size());
		verts.resize(vsz);
		normals.resize(vsz);
		inds.resize(vsz);
	}

	VisSystem::SetMsg("Loading Atoms");
	colors.reserve(mpt);
	matrices.reserve(mpt);
	imatrices.reserve(mpt);
	indents.reserve(mpt);
	for (auto& dl : ParGraphics::drawLists) {
		float scl;
		if (dl.second.second == 1) scl = 0.00f;
		else if (dl.second.second == 0x0f) scl = 0.008f;
		else if (dl.second.second == 2) scl = 0.02f * ParGraphics::radScl;
		else scl = 0.17f * ParGraphics::radScl;

		for (uint i = 0; i < dl.second.first; i++) {
			const uint id = dl.first + i;
			if (!ParGraphics::useGradCol || !attr->size()) {
				colors.push_back(Particles::_colorPallete[Particles::colors[id]]);
			}
			else {
				auto col = Color::HueBaseCol(Clamp((1 - (float)(*attr)[id]), 0.f, 1.f) * 0.6667f);
				colors.push_back(col);
			}
			matrices.push_back(RR::MatFunc::Translate(Particles::poss[id]) * RR::scale(RR::float3(scl, scl, scl) * Particles::radii[id]));
			imatrices.push_back(RR::scale(RR::float3(1/scl, 1/scl, 1/scl)) * RR::MatFunc::Translate(-Particles::poss[id]));
		}
	}
	indents.resize(mp, 0);

	VisSystem::SetMsg("Loading Bonds");
	for (auto& dl : ParGraphics::drawListsB) {
		colors.resize(mp + dl.second.first*2);
		matrices.resize(mp + dl.second.first*2);
		imatrices.resize(mp + dl.second.first*2);
#pragma omp parallel for
		for (int i = 0; i < (int)dl.second.first; i++) {
			const uint id = dl.first + i;
			auto ii = Particles::conns.ids[id];
			auto p0 = (Vec3)Particles::poss[ii.x];
			auto p1 = (Vec3)Particles::poss[ii.y];
			auto dp = p1 - p0;
			auto len = glm::length(dp) / 2;
			Mat4x4 mat2 = QuatFunc::ToMatrix(QuatFunc::LookAt(dp)) * glm::scale(Vec3(1, 1, len));

			colors[mp + i * 2] = colors[ii.x];
			matrices[mp + i * 2] = RR::MatFunc::Glm2RR(glm::translate(p0) * mat2);
			imatrices[mp + i * 2] = RR::inverse(matrices[mp + i * 2]);
			colors[mp + i * 2 + 1] = colors[ii.y];
			matrices[mp + i * 2 + 1] = RR::MatFunc::Glm2RR(glm::translate((p0 + p1) * 0.5f) * mat2);
			imatrices[mp + i * 2 + 1] = RR::inverse(matrices[mp + i * 2 + 1]);
		}
	}
	indents.resize(mp + mp2, _indents[1]);

	for (auto& s : shapes2) {
		colors.push_back(white());
		matrices.push_back(RR::matrix());
		indents.push_back(_indents[2]);
	}

	VisSystem::SetMsg("Creating Shapes");
	
	size_t shpId = 0;
	auto _shp = shapes[0];
	RR::Shape* shape0 = api->CreateMesh((float*)_shp->vertices.data(), _shp->vertCount, sizeof(Vec3), _shp->triangles.data(), 0, nullptr, _shp->triCount);
	for (int id = 0; id < mp; ++id) {
		auto shp = (!id) ? shape0 : api->CreateInstance(shape0);
		shp->SetTransform(matrices[shpId], imatrices[shpId]);
		shp->SetId(shpId++);
		api->AttachShapeUnchecked(shp);
	}

	_shp = shapes[1];
	shape0 = api->CreateMesh((float*)_shp->vertices.data(), _shp->vertCount, sizeof(Vec3), _shp->triangles.data(), 0, nullptr, _shp->triCount);
	for (int id = 0; id < mp2; id++) {
		auto shp = (!id) ? shape0 : api->CreateInstance(shape0);
		shp->SetTransform(matrices[shpId], imatrices[shpId]);
		shp->SetId(shpId++);
		api->AttachShapeUnchecked(shp);
	}

	for (auto& s : shapes2) {
		auto shp = api->CreateMesh((float*)s.vertices.data(), s.vertCount, sizeof(Vec3), s.triangles.data(), 0, nullptr, s.triCount);
		shp->SetId(shpId++);
		api->AttachShapeUnchecked(shp);
	}

	VisSystem::SetMsg("Generating Ray Structure");
	try {
		api->Commit();
	}
	catch (std::runtime_error& err) {
		 Debug::Warning("RayTracer::CommitRR", err.what());
		 return;
	}

	g_positions = CLWBuffer_New(float, verts.size(), verts.data());
	g_normals = CLWBuffer_New(float, normals.size(), normals.data());
	g_indices = CLWBuffer_New(int, inds.size(), inds.data());
	g_colors = CLWBuffer_New(Vec4, mpt, colors.data());
	g_indent = CLWBuffer_New(int, mpt, indents.data());
	g_matrices = CLWBuffer_New(RR::matrix, mpt, matrices.data());
}

void RayTracer::SetSky() {
	VisSystem::SetMsg("Loading Background");
	auto d = hdr::read_hdr((IO::path + "backgrounds/" + ParGraphics::reflNms[ParGraphics::reflId] + "/specular.hdr").c_str(), &bgw, &bgh);
	std::vector<float> dv(bgw * bgh * 3);
	hdr::to_float(d, bgw, bgh, dv.data());
	bg_buf = CLWBuffer<float>::Create(context, CL_MEM_READ_ONLY, 3 * bgw * bgh, dv.data());
	delete[](d);
}

CLWBuffer<RR::ray> RayTracer::GeneratePrimaryRays() {
#define karg(var) kernel.SetArg(i++, var)
	CLWBuffer<RR::ray> ray_buf = CLWBuffer<RR::ray>::Create(context, CL_MEM_READ_WRITE, Display::width * Display::height);

	if (ParGraphics::Eff::useDof) {
		struct Camera {
			Mat4x4 ip = glm::inverse(ParGraphics::lastMVP);
			Vec4 pos;
			Vec4 fwd;
			Vec2 zcap = Vec2(0.001f, 1000.f);
		} cam;
		cam.pos = cam.ip * Vec4(0, 0, -1, 1);
		cam.pos /= cam.pos.w;
		cam.fwd = cam.ip * Vec4(0, 0, 1, 1);
		cam.fwd /= cam.fwd.w;
		cam.fwd -= cam.pos;
		cam.fwd.w = 0;
		cam.fwd = glm::normalize(cam.fwd);
		cam.ip = glm::transpose(cam.ip);

		CLWBuffer<Camera> cam_buf = CLWBuffer<Camera>::Create(context, CL_MEM_READ_ONLY, 1, &cam);
		
		float scl = std::pow(2.f, ParGraphics::rotScale);

		auto kernel = program.GetKernel("GenRays_Dof");
		
		int i = 0;
		karg(ray_buf);
		karg(cam_buf);
		karg(Display::width);
		karg(Display::height);
		karg(ParGraphics::Eff::dofDepth / scl);
		karg(ParGraphics::Eff::dofAper / 20 / scl);
		karg((cl_int)milliseconds());

		// Run generation kernel
		size_t gs[] = { static_cast<size_t>((Display::width + 7) / 8 * 8), static_cast<size_t>((Display::height + 7) / 8 * 8) };
		size_t ls[] = { 8, 8 };
		rendEvent2 = context.Launch2D(0, gs, ls, kernel);
		context.Flush(0);

		return ray_buf;
	}
	else {
		struct Camera {
			Mat4x4 ip = glm::transpose(glm::inverse(ParGraphics::lastMVP));
			Vec2 zcap = Vec2(0.001f, 1000.f);
		} cam;
		cl_float4 patch = { patchPos.x, patchPos.y, patchSize.x, patchSize.y };
		
		CLWBuffer<Camera> cam_buf = CLWBuffer<Camera>::Create(context, CL_MEM_READ_ONLY, 1, &cam);
		
		auto kernel = program.GetKernel("GenRays_Pin");
		
		int i = 0;
		karg(ray_buf);
		karg(cam_buf);
		karg(Display::width);
		karg(Display::height);
		karg(patch);

		// Run generation kernel
		size_t gs[] = { ((patchSize.x + 7) / 8) * 8, ((patchSize.y + 7) / 8) * 8 };
		size_t ls[] = { 8, 8 };
		try {
			rendEvent2 = context.Launch2D(0, gs, ls, kernel);
		}
		catch (CLWException& e) {
			Debug::Warning("RayTracer", e.what() + clErrorString(e.errcode_));
			return ray_buf;
		}
		context.Flush(0);

		return ray_buf;
	}
#undef karg
}

void RayTracer::_Refine() {
	//patchSize = { Display::width, Display::height };
	Int2 _patchSize = patchSize = { 64, 64 };
	patchPos = Int2();

	SetSky();
	SetObjs();

	int wh = patchSize.x * patchSize.y;
	accum = CLWBuffer<float>::Create(context, CL_MEM_READ_WRITE, 3 * wh);
	
	int patchi = 0;
	int patchtotx = (Display::width + _patchSize.x - 1) / _patchSize.x;
	int patchtoty = (Display::height + _patchSize.y - 1) / _patchSize.y;
	int patchtot = (patchtotx - 1) * (patchtoty - 1);

	VisSystem::SetMsg("Tracing patch 0");

	while (!kill) {
		if (scene_dirty) {
			scene_dirty = false;
			samples = 0;
			patchi = 0;
			patchPos = Int2();
			patchSize = _patchSize;
			wh = patchSize.x * patchSize.y;
			VisSystem::SetMsg("Tracing patch 0");
		}
		else if (samples > maxSamples) {
			continue;
		}
		//VisSystem::SetMsg("Tracing sample " + std::to_string(samples));
		ray_buffer_cl = GeneratePrimaryRays();
		rendEvent2.Wait();
		isect_buffer_cl = CLWBuffer<RR::Intersection>::Create(context, CL_MEM_READ_WRITE, wh);
		isect_buffer = CreateFromOpenClBuffer(api, isect_buffer_cl);
		for (rendBounce = 0; rendBounce < maxRefl; rendBounce++) {
			ray_buffer = CreateFromOpenClBuffer(api, ray_buffer_cl);
			api->QueryIntersection(ray_buffer, wh, isect_buffer, nullptr, &rendEvent);
			rendEvent->Wait();
			if (!rendBounce) {
				out_buff = CLWBuffer<float>::Create(context, CL_MEM_WRITE_ONLY, 4 * wh);
				col_buff = CLWBuffer<float>::Create(context, CL_MEM_READ_WRITE, 4 * wh);
			}
			ShadeKernel(out_buff, isect_buffer_cl, col_buff, ray_buffer_cl, std::max(samples, 1), !rendBounce);
			rendEvent2.Wait();
			delete(ray_buffer);
		}

		++samples;
		void* ps = clEnqueueMapBuffer(queue, (cl_mem)out_buff, true, CL_MAP_READ, 0, 4 * wh * sizeof(float), 0, NULL, NULL, NULL);
		
		std::memcpy(pixels.data(), ps, 4 * wh * sizeof(float));
		
		clEnqueueUnmapMemObject(queue, (cl_mem)out_buff, ps, 0, NULL, NULL);
		delete(isect_buffer);
		
		if (samples == maxSamples) {
			if (patchi == patchtot) {
				VisSystem::SetMsg("Denoising...");
				Denoise();
				samples++;
				VisSystem::SetMsg("RayTracing Complete.");
			}
			else {
				patchi++;
				patchPos.x = (patchi % patchtotx) * _patchSize.x;
				patchPos.y = (patchi / patchtotx) * _patchSize.y;
				patchSize.x = std::min(Display::width - patchPos.x, _patchSize.x);
				patchSize.y = std::min(Display::height - patchPos.y, _patchSize.y);
				wh = patchSize.x * patchSize.y;
				samples = 0;
				VisSystem::SetMsg("Tracing patch " + std::to_string(patchi));
			}
		}
	}
}

void RayTracer::ShadeKernel(CLWBuffer<float> out_buff, const CLWBuffer<RR::Intersection>& isect, CLWBuffer<float>& col_buff, CLWBuffer<RR::ray>& ray_buff, const int smps, const bool isprim)
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
	karg(patchSize.x);
	karg(patchSize.y);
	karg(out_buff);
	karg(col_buff);
	karg(ray_buff);
	karg(cl_int(rand() % RAND_MAX));
	karg(accum);
	karg(smps);
	karg(ParGraphics::specStr);
	karg(0.f);
	karg(ParGraphics::reflTr);
	karg(ParGraphics::reflIor);
	karg(bg_buf);
	karg(bgw);
	karg(bgh);
	karg(ParGraphics::reflStr);
	karg(ParGraphics::bgMul);
#undef karg
	
	// Run generation kernel
	size_t gs[] = { ((patchSize.x + 7) / 8) * 8, ((patchSize.y + 7) / 8) * 8 };
	size_t ls[] = { 8, 8 };
	try {
		rendEvent2 = context.Launch2D(0, gs, ls, kernel);
	}
	catch (CLWException& e) {
		Debug::Warning("RayTracer", e.what() + clErrorString(e.errcode_));
		return;
	}
	context.Flush(0);
}
