#include "raytracer.h"
#include "hdr.h"
#include "md/Particles.h"
#include "md/ParMenu.h"
#include "utils/dialog.h"
#include "vis/pargraphics.h"
#include "ui/ui_ext.h"
#ifdef PLATFORM_OSX
#include <OpenCL/cl_gl_ext.h>
#include <OpenGL/CGLDevice.h>
#include <OpenGL/CGLCurrent.h>
#include <OpenCL/cl_gl.h>
#else
#include <CL/cl_gl.h>
#ifdef PLATFORM_LNX
#define Time Time_XII
#define Font Font_XII
#define Display Display_XII
#include <GL/glx.h>
#undef Time
#undef Font
#undef Display
#endif
#endif

GLuint RayTracer::resTex = 0;

bool RayTracer::Init() {
	return false;
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
	Engine::DrawQuad(expandPos - 149, off + 17, 148, 17 * 2 + 2, white(0.9f, 0.1f));
	off++;
	prvRes = UI2::Slider(expandPos - 147, off + 17, 147, "Quality", 0.1f, 1, prvRes, std::to_string(int(prvRes * 100)) + "%");
	prvSmp = TryParse(UI2::EditText(expandPos - 147, off + 17 * 2, 147, "Samples", std::to_string(prvSmp)), 50U);

	off += 17 * 3 * 2;

	UI::Label(expandPos - 148, off, 12, "Background", white());
	Engine::DrawQuad(expandPos - 149, off + 17, 148, 17 * 2 + 2, white(0.9f, 0.1f));
	off++;
	UI2::File(expandPos - 147, off + 17, 147, "File", bgName, [](std::vector<string> res) {
		SetBg(res[0]);
		_cntt = 0;
	});
	SV(info.str, str) = UI2::Slider(expandPos - 147, off + 17 * 2, 147, "Strength", 0, 5, info.str);

	off += 17 * 3 + 2;

	UI::Label(expandPos - 148, off, 12, "Material", white());
	Engine::DrawQuad(expandPos - 149, off + 17, 148, 17 * 2 + 2, white(0.9f, 0.1f));
	off++;
	SV(mt.specular, spc) = UI2::Slider(expandPos - 147, off + 17, 147, "Specular", 0, 1, mt.specular);
	SV(mt.gloss, gls) = UI2::Slider(expandPos - 147, off + 17 * 2, 147, "Gloss", 0, 1, mt.gloss);

	if ((info.str != str) || (mt.specular != spc) || mt.gloss != gls) {
		mt.rough = (1-mt.gloss)*0.5f;
		_cntt = 0;
	}
	*/
}
