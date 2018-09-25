#include "raytracer.h"
#include "hdr.h"
#include "kernel.h"

GLuint RayTracer::resTex = 0;

CLWContext RayTracer::context;
CLWProgram RayTracer::program;

CLWCommandQueue RayTracer::queue;
RR::IntersectionApi* RayTracer::api;

bool RayTracer::Init() {
	std::vector<CLWPlatform> platforms;
	CLWPlatform::CreateAllPlatforms(platforms);

	if (platforms.size() == 0)
	{
		Debug::Warning("RayTracer", "No OpenCL platforms installed.");
		return false;
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
		Debug::Warning("RayTracer", "Cannot find a GPU context!");
		return false;
	}
	const char* kBuildopts(" -cl-mad-enable -cl-fast-relaxed-math -cl-std=CL1.2 -I.");

	try {
		program = CLWProgram::CreateFromSource(ocl::raykernel, ocl::raykernel_sz, kBuildopts, context);
	}
	catch (CLWException& e) {
		OHNO("Raytracer", + std::string(e.what()));
		return false;
	}

	auto id = context.GetDevice(0).GetID();
	queue = context.GetCommandQueue(0);
	
	return false;
	try {
		//api = RR::CreateFromOpenClContext(context, id, queue, IO::path);
	}
	catch (std::runtime_error& err) {
		 Debug::Warning("RayTracer::InitRR", err.what());
	}
	return true;
}

void RayTracer::Clear() {
	
}

void RayTracer::SetScene() {
	
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
