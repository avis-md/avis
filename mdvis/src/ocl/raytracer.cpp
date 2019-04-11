#include "raytracer.h"
#include "hdr.h"
#include "kernel.h"
#include "vis/pargraphics.h"
#include "denoise.h"

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

uint RayTracer::bgw, RayTracer::bgh;
int RayTracer::maxRefl = 2;

GLuint RayTracer::resTex = 0;
CLWBuffer<float> RayTracer::accum;
int RayTracer::samples = 0, RayTracer::maxSamples = 2000;

CLWContext RayTracer::context;
CLWProgram RayTracer::program;
CLWCommandQueue RayTracer::queue;
RR::IntersectionApi* RayTracer::api;

RayTracer::REND_STEP RayTracer::rendStep = RayTracer::REND_STEP::IDLE;
int RayTracer::rendBounce = 0;
RR::Event* RayTracer::rendEvent;
CLWEvent RayTracer::rendEvent2;

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
		program = CLWProgram::CreateFromSource(ocl::raykernel, ocl::raykernel_sz, kBuildopts, context);
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
	const int wh = Display::width * Display::height;
	accum = CLWBuffer<float>::Create(context, CL_MEM_READ_WRITE, 3 * wh);

	SetSky();
	SetObjs();

	glGenTextures(1, &resTex);
	glBindTexture(GL_TEXTURE_2D, resTex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, Display::width, Display::height, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
	SetTexParams<>();
	glBindTexture(GL_TEXTURE_2D, 0);
}

void RayTracer::Refine() {
	const int wh = Display::width * Display::height;

	if (Scene::dirty) {
		samples = 0;
	}

	switch (rendStep) {
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
				glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, Display::width, Display::height, GL_RGBA, GL_FLOAT, pixels);
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

void RayTracer::DrawMenu() {

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
		
		auto kernel = program.GetKernel("GenRays_Dof");
		
		int i = 0;
		karg(ray_buf);
		karg(cam_buf);
		karg(Display::width);
		karg(Display::height);
		karg(ParGraphics::Eff::dofDepth);
		karg(ParGraphics::Eff::dofAper / 20);
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
		
		CLWBuffer<Camera> cam_buf = CLWBuffer<Camera>::Create(context, CL_MEM_READ_ONLY, 1, &cam);
		
		auto kernel = program.GetKernel("GenRays_Pin");
		
		int i = 0;
		karg(ray_buf);
		karg(cam_buf);
		karg(Display::width);
		karg(Display::height);

		// Run generation kernel
		size_t gs[] = { static_cast<size_t>((Display::width + 7) / 8 * 8), static_cast<size_t>((Display::height + 7) / 8 * 8) };
		size_t ls[] = { 8, 8 };
		rendEvent2 = context.Launch2D(0, gs, ls, kernel);
		context.Flush(0);

		return ray_buf;
	}
#undef karg
}

#include "asset/tetrahedron.h"
#include "asset/tube.h"

void RayTracer::SetObjs() {
	Debug::Message("RayTracer", "Loading Meshes");
	const uint mp = Particles::particleSz;

	Tetrahedron tet = Tetrahedron();
	for (int a = 0; a < 4; a++)
		tet.Subdivide();
	tet.ToSphere(0.15f);

	Tube tub = Tube(16, 10, 50);

	std::vector<_Mesh*> shapes = { &tet, &tub };
	std::vector<int> _indents;

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

	Debug::Message("RayTracer", "Loading Atoms");
	colors.reserve(mp);
	indents.push_back(_indents[1]);
	matrices.reserve(mp);
	imatrices.reserve(mp);
	for (int id = 0; id < mp; ++id) {
		if (!ParGraphics::useGradCol || !attr->size()) {
			colors.push_back(Particles::_colorPallete[Particles::colors[id]]);
		}
		else {
			auto col = Color::HueBaseCol(Clamp((1 - (float)(*attr)[id]), 0.f, 1.f) * 0.6667f);
			colors.push_back(col);
		}

		matrices.push_back(RR::MatFunc::Translate(Particles::poss[id]));
		imatrices.push_back(RR::MatFunc::Translate(-Particles::poss[id]));
	}

	colors.push_back(white());
	matrices.push_back(RR::matrix());

	auto _shp = shapes[0];
	RR::Shape* shape0 = api->CreateMesh((float*)_shp->vertices.data(), _shp->vertCount * 3, sizeof(Vec3), _shp->triangles.data(), 0, nullptr, _shp->triCount);
	for (int id = 0; id < mp; ++id) {
		auto shp = (!id) ? shape0 : api->CreateInstance(shape0);
		//shp->SetTransform(matrices[id], RR::inverse(matrices[id]));
		shp->SetTransform(matrices[id], imatrices[id]);
		shp->SetId(id);
		api->AttachShapeUnchecked(shp);
	}

	/*_shp = shapes[1];
	shape0 = api->CreateMesh((float*)_shp->vertices.data(), _shp->vertCount * 3, sizeof(Vec3), _shp->triangles.data(), 0, nullptr, _shp->triCount);
	shape0->SetId(mp);
	api->AttachShape(shape0);*/

	Debug::Message("RayTracer", "Committing");
	api->Commit();

	g_positions = CLWBuffer<float>::Create(context, CL_MEM_READ_ONLY, verts.size(), verts.data());
	g_normals = CLWBuffer<float>::Create(context, CL_MEM_READ_ONLY, normals.size(), normals.data());
	g_indices = CLWBuffer<int>::Create(context, CL_MEM_READ_ONLY, inds.size(), inds.data());
	g_colors = CLWBuffer<Vec4>::Create(context, CL_MEM_READ_ONLY, mp + 1, colors.data());
	g_indent = CLWBuffer<int>::Create(context, CL_MEM_READ_ONLY, mp + 1, indents.data());
	g_matrices = CLWBuffer<RR::matrix>::Create(context, CL_MEM_READ_ONLY, mp + 1, matrices.data());
}

void RayTracer::SetSky() {
	Debug::Message("RayTracer", "Loading HDRI");
	auto d = hdr::read_hdr((IO::path + "backgrounds/" + ParGraphics::reflNms[ParGraphics::reflId] + "/specular.hdr").c_str(), &bgw, &bgh);
	std::vector<float> dv(bgw * bgh * 3);
	hdr::to_float(d, bgw, bgh, dv.data());
	bg_buf = CLWBuffer<float>::Create(context, CL_MEM_READ_ONLY, 3 * bgw * bgh, dv.data());
	delete[](d);
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
	karg(Display::width);
	karg(Display::height);
	karg(out_buff);
	karg(col_buff);
	karg(ray_buff);
	karg(cl_int(rand() % RAND_MAX));
	karg(accum);
	karg(smps);
	karg(ParGraphics::specStr);
	karg(0.05f);
	karg(bg_buf);
	karg(bgw);
	karg(bgh);
	karg(ParGraphics::reflStr);
	karg(ParGraphics::bgMul);
#undef karg
	
	// Run generation kernel
	size_t gs[] = { static_cast<size_t>((Display::width + 7) / 8 * 8), static_cast<size_t>((Display::height + 7) / 8 * 8) };
	size_t ls[] = { 8, 8 };
	rendEvent2 = context.Launch2D(0, gs, ls, kernel);
	context.Flush(0);
}