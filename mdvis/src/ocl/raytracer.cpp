#include "raytracer.h"
#include "hdr.h"
#include "kernel.h"
#include "tmp/tiny_obj_loader.h"

namespace TO = tinyobj;

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
	CLWPlatform::CreateAllPlatforms(platforms);

	if (platforms.size() == 0)
	{
		FAIL("No OpenCL platforms installed.");
	}

	for (int i = 0; i < platforms.size(); ++i) {
		for (int d = 0; d < (int)platforms[i].GetDeviceCount(); ++d) {
			if (platforms[i].GetDevice(d).GetType() == CL_DEVICE_TYPE_GPU) {
				context = CLWContext::Create(platforms[i].GetDevice(d));
				break;
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

void RayTracer::Clear() {
	accum = CLWBuffer<float>();

	resTex = 0;
}

std::vector<TO::shape_t> g_objshapes;
std::vector<TO::material_t> g_objmaterials;

CLWBuffer<float> bg_buf;
CLWBuffer<float> g_positions;
CLWBuffer<float> g_normals;
CLWBuffer<int> g_indices;
CLWBuffer<float> g_colors;
CLWBuffer<int> g_indent;

void RayTracer::SetScene() {
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

	CLWBuffer<RR::ray> ray_buffer_cl = GeneratePrimaryRays();
	RR::Buffer* ray_buffer = CreateFromOpenClBuffer(api, ray_buffer_cl);
	// Intersection data
	CLWBuffer<RR::Intersection> isect_buffer_cl = CLWBuffer<RR::Intersection>::Create(context, CL_MEM_READ_WRITE, wh);
	RR::Buffer* isect_buffer = CreateFromOpenClBuffer(api, isect_buffer_cl);

	// Intersection
	api->QueryIntersection(ray_buffer, wh, isect_buffer, nullptr, nullptr);

	float* zeros = new float[4 * wh]{};

	auto out_buff = CLWBuffer<byte>::Create(context, CL_MEM_WRITE_ONLY, 4 * wh, zeros);
	auto col_buff = CLWBuffer<float>::Create(context, CL_MEM_READ_WRITE, 4 * wh, zeros);

	delete[](zeros);

	// Shading
	RR::Event* e = nullptr;

	/*
	for (int a = 0; a < 2; a++) {
		ShadeKernel(out_buff, isect_buffer_cl, col_buff, ray_buffer_cl, 1 + samples);
		delete(ray_buffer);
		ray_buffer = CreateFromOpenClBuffer(api, ray_buffer_cl);
		api->QueryIntersection(ray_buffer, wh, isect_buffer, nullptr, &e);
		e->Wait();
	}*/
	ShadeKernel(out_buff, isect_buffer_cl, col_buff, ray_buffer_cl, ++samples);
	delete(ray_buffer);

	void* pixels = clEnqueueMapBuffer(queue, (cl_mem)out_buff, true, CL_MAP_READ, 0, 4 * wh, 0, NULL, NULL, NULL);

	// Update texture data
	glBindTexture(GL_TEXTURE_2D, resTex);
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, Display::width, Display::height, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
	glBindTexture(GL_TEXTURE_2D, NULL);

	clEnqueueUnmapMemObject(queue, (cl_mem)out_buff, pixels, 0, NULL, NULL);

	delete(isect_buffer);
}

void RayTracer::Render() {
	
}

void RayTracer::DrawMenu() {
	/*
#define SV(vl, b) auto b = vl; vl

	auto& expandPos = ParMenu::expandPos;
	auto& mt = info.mat;

	if (Engine::Button(expandPos - 148, 20, 146, 16, resTex ? Vec4(0.4f, 0.2f, 0.2f, 1) : Vec4(0.2f, 0.4f, 0.2f, 1), resTex ? "Disable (Shift-X)" : "Enable (Shift-X)", 12, white(), true) == MOUSE_RELEASE) {
		if (resTex) Clear();
		else SetScene();
	}

	float off = 17 * 3 + 1;
	if (resTex) {
		UI::Label(expandPos - 148, 17 * 2, 12, "Samples: " + std::to_string(_cntt), white(0.5f));
		off += 17;
	}

	UI::Label(expandPos - 148, off, 12, "Preview", white());
	UI::Quad(expandPos - 149, off + 17, 148, 17 * 2 + 2, white(0.9f, 0.1f));
	off++;
	prvRes = UI2::Slider(expandPos - 147, off + 17, 147, "Quality", 0.1f, 1, prvRes, std::to_string(int(prvRes * 100)) + "%");
	prvSmp = TryParse(UI2::EditText(expandPos - 147, off + 17 * 2, 147, "Samples", std::to_string(prvSmp)), 50U);

	off += 17 * 3 * 2;

	UI::Label(expandPos - 148, off, 12, "Background", white());
	UI::Quad(expandPos - 149, off + 17, 148, 17 * 2 + 2, white(0.9f, 0.1f));
	off++;
	UI2::File(expandPos - 147, off + 17, 147, "File", bgName, [](std::vector<std::string> res) {
		SetBg(res[0]);
		_cntt = 0;
	});
	SV(info.str, str) = UI2::Slider(expandPos - 147, off + 17 * 2, 147, "Strength", 0, 5, info.str);

	off += 17 * 3 + 2;

	UI::Label(expandPos - 148, off, 12, "Material", white());
	UI::Quad(expandPos - 149, off + 17, 148, 17 * 2 + 2, white(0.9f, 0.1f));
	off++;
	SV(mt.specular, spc) = UI2::Slider(expandPos - 147, off + 17, 147, "Specular", 0, 1, mt.specular);
	SV(mt.gloss, gls) = UI2::Slider(expandPos - 147, off + 17 * 2, 147, "Gloss", 0, 1, mt.gloss);

	if ((info.str != str) || (mt.specular != spc) || mt.gloss != gls) {
		mt.rough = (1-mt.gloss)*0.5f;
		_cntt = 0;
	}
	*/
}

CLWBuffer<RR::ray> RayTracer::GeneratePrimaryRays() {
	struct Cam
	{
		// Camera coordinate frame
		Vec3 forward = Vec3(0, 0, 1);
		Vec3 up = Vec3(0, 1, 0);
		Vec3 p = Vec3(0, 1, 5);
		// Near and far Z
		Vec2 zcap = Vec2(1, 1000);
	} cam = Cam();
	CLWBuffer<Cam> cambuf = CLWBuffer<Cam>::Create(context, CL_MEM_READ_ONLY, 1, &cam);

	//run kernel
	CLWBuffer<RR::ray> primRays = CLWBuffer<RR::ray>::Create(context, CL_MEM_READ_WRITE, Display::width * Display::height);
	CLWKernel kernel = program.GetKernel("GeneratePerspectiveRays");
	kernel.SetArg(0, primRays);
	kernel.SetArg(1, cambuf);
	kernel.SetArg(2, Display::width);
	kernel.SetArg(3, Display::height);

	// Run generation kernel
	size_t gs[] = { static_cast<size_t>((Display::width + 7) / 8 * 8), static_cast<size_t>((Display::height + 7) / 8 * 8) };
	size_t ls[] = { 8, 8 };
	context.Launch2D(0, gs, ls, kernel);
	context.Flush(0);

	return primRays;
}

void RayTracer::SetObjs() {
	auto res = TO::LoadObj(g_objshapes, g_objmaterials, (IO::path + "res/sharo.obj").c_str(), (IO::path + "res/").c_str());
	if (res != "")
	{
		throw std::runtime_error(res);
	}

	std::vector<float> verts;
	std::vector<float> normals;
	std::vector<int> inds;
	std::vector<float> colors;
	std::vector<int> indents;
	int indent = 0;
	for (int id = 0; id < g_objshapes.size(); ++id)
	{
		const TO::mesh_t& mesh = g_objshapes[id].mesh;
		verts.insert(verts.end(), mesh.positions.begin(), mesh.positions.end());
		normals.insert(normals.end(), mesh.normals.begin(), mesh.normals.end());
		inds.insert(inds.end(), mesh.indices.begin(), mesh.indices.end());
		for (int mat_id : mesh.material_ids)
		{
			const TO::material_t& mat = g_objmaterials[mat_id];
			colors.push_back(mat.diffuse[0]);
			colors.push_back(mat.diffuse[1]);
			colors.push_back(mat.diffuse[2]);
		}

		verts.resize(mesh.indices.size() * 3, 0.0f);
		normals.resize(mesh.indices.size() * 3, 0.0f);

		indents.push_back(indent);
		indent += mesh.indices.size();
	}
	g_positions = CLWBuffer<float>::Create(context, CL_MEM_READ_ONLY, verts.size(), verts.data());
	g_normals = CLWBuffer<float>::Create(context, CL_MEM_READ_ONLY, normals.size(), normals.data());
	g_indices = CLWBuffer<int>::Create(context, CL_MEM_READ_ONLY, inds.size(), inds.data());
	g_colors = CLWBuffer<float>::Create(context, CL_MEM_READ_ONLY, colors.size(), colors.data());
	g_indent = CLWBuffer<int>::Create(context, CL_MEM_READ_ONLY, indents.size(), indents.data());

	unsigned int _w, _h;
	auto d = hdr::read_hdr((IO::path + "res/refl.hdr").c_str(), &_w, &_h);
	float* dv = new float[_w*_h * 3];
	hdr::to_float(d, _w, _h, dv);
	bg_buf = CLWBuffer<float>::Create(context, CL_MEM_READ_ONLY, 3 * _w * _h, dv);
	delete[](d);
	delete[](dv);

	for (int id = 0; id < g_objshapes.size(); ++id)
	{
		auto& objshape = g_objshapes[id];
		float* vertdata = objshape.mesh.positions.data();
		int nvert = objshape.mesh.positions.size() / 3;
		int* indices = objshape.mesh.indices.data();
		int nfaces = objshape.mesh.indices.size() / 3;
		RR::Shape* shape = api->CreateMesh(vertdata, nvert, 3 * sizeof(float), indices, 0, nullptr, nfaces);

		assert(shape != nullptr);
		api->AttachShape(shape);
		shape->SetId(id);
	}

	api->Commit();
}

void RayTracer::ShadeKernel(CLWBuffer<byte> out_buff, const CLWBuffer<RR::Intersection>& isect, CLWBuffer<float>& col_buff, CLWBuffer<RR::ray>& ray_buff, const int smps)
{
	//run kernel
	CLWKernel kernel = program.GetKernel("Shading");
	kernel.SetArg(0, g_positions);
	kernel.SetArg(1, g_normals);
	kernel.SetArg(2, g_indices);
	kernel.SetArg(3, g_colors);
	kernel.SetArg(4, g_indent);
	kernel.SetArg(5, isect);
	kernel.SetArg(6, 1);
	kernel.SetArg(7, Display::width);
	kernel.SetArg(8, Display::height);
	kernel.SetArg(9, out_buff);
	kernel.SetArg(10, col_buff);
	kernel.SetArg(11, ray_buff);
	kernel.SetArg(12, cl_int(rand() % RAND_MAX));
	kernel.SetArg(13, accum);
	kernel.SetArg(14, smps);
	kernel.SetArg(15, bg_buf);

	// Run generation kernel
	size_t gs[] = { static_cast<size_t>((Display::width + 7) / 8 * 8), static_cast<size_t>((Display::height + 7) / 8 * 8) };
	size_t ls[] = { 8, 8 };
	context.Launch2D(0, gs, ls, kernel);
	context.Flush(0);
}