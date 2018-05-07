#include "ChokoLait.h"
#include "md/Particles.h"
#include "md/Gromacs.h"
//#define NPY_NO_DEPRECATED_API NPY_1_7_API_VERSION
//#include <numpy/arrayobject.h>

#include "ui/icons.h"
#include "py/PyWeb.h"
#include "md/ParMenu.h"
#include "vis/pargraphics.h"
#include "vis/system.h"

float camz = 10;
Vec3 center;
float rw = 0, rz = 0;
float repx, repy, repz;

Gromacs* gro;
Shader* shad, *shad2;
Background* bg;
Font* font;
GLuint emptyVao;

bool drawMesh;

void rendFunc() {
	/*
	MVP::Switch(false);
	MVP::Clear();
	float csz = cos(-rz*deg2rad);
	float snz = sin(-rz*deg2rad);
	float csw = cos(rw*deg2rad);
	float snw = sin(rw*deg2rad);
	Mat4x4 mMatrix = Mat4x4(1, 0, 0, 0, 0, csw, snw, 0, 0, -snw, csw, 0, 0, 0, 0, 1) * Mat4x4(csz, 0, -snz, 0, 0, 1, 0, 0, snz, 0, csz, 0, 0, 0, 0, 1);
	MVP::Mul(mMatrix);
	//MVP::Translate(-gro->boundingBox.x / 2, -gro->boundingBox.y / 2, -gro->boundingBox.z / 2);
	MVP::Translate(center);
	*/
	ParGraphics::Rerender();
}

void updateFunc() {
	ParGraphics::Update();

	PyWeb::Update();
}

void paintfunc2() {
	if (PyWeb::drawFull)
		PyWeb::Draw();
	else {
		ParMenu::Draw();
		PyWeb::DrawSide();
	}
	VisSystem::DrawBar();

	ParGraphics::hlIds.clear();
	if (VisSystem::InMainWin(Input::mousePos)) {
		auto id = ChokoLait::mainCamera->GetIdAt((uint)Input::mousePos.x, (uint)Input::mousePos.y);
		if (id) {
			ParGraphics::hlIds.push_back(id);
			Engine::DrawQuad(Input::mousePos.x + 14, Input::mousePos.y + 2, 120, 60, white(0.8f, 0.1f));
			UI::Label(Input::mousePos.x + 14, Input::mousePos.y + 2, 12, "Particle " + std::to_string(id - 1), font, white());
			UI::Label(Input::mousePos.x + 14, Input::mousePos.y + 17, 12, &Particles::particles_ResName[id * PAR_MAX_NAME_LEN], PAR_MAX_NAME_LEN, font, white());
			UI::Label(Input::mousePos.x + 14, Input::mousePos.y + 32, 12, &Particles::particles_Name[id * PAR_MAX_NAME_LEN], PAR_MAX_NAME_LEN, font, white());
		}
	}
}

int main(int argc, char **argv)
{
	ChokoLait::Init(800, 800);
	bg = new Background(IO::path + "/refl.hdr");
	font = new Font(IO::path + "/arimo.ttf", ALIGN_TOPLEFT);
	
	VisSystem::font = font;

	Icons::Init();
	Particles::Init();
	ParGraphics::Init();
	
	PyReader::Init();
	PyNode::Init();
	PyNode::font = font;

	PyBrowse::Scan();
	
	PyWeb::Init();
	ParMenu::font = font;

	/*
	while (ChokoLait::alive()) {
		ChokoLait::Update();
		ChokoLait::Paint(nullptr, paintfunc2);
	}
	*/
	//*
	auto& set = Scene::active->settings;
	set.sky = bg;
	set.skyStrength = 1.5f;
	set.skyBrightness = 0;
	//Gromacs::LoadFiles();
	Gromacs::Read(IO::path + "/md.gro");
	ParGraphics::UpdateDrawLists();

	glEnable(GL_PROGRAM_POINT_SIZE);

	Display::Resize(800, 600, false);

	auto lastMillis = Time::millis;
	bool dirty = false;

	ChokoLait::mainCamera->object->transform.localPosition(Vec3(0, 0, -1));

	while (ChokoLait::alive()) {
		ChokoLait::Update(updateFunc);
		//ChokoLait::mainCamera->object->transform.localPosition(Vec3(0, 0, -camz));
		//ChokoLait::mainCamera->object->transform.localPosition(-ParGraphics::rotCenter);

		dirty = Scene::dirty;
		//ChokoLait::Paint(nullptr, paintfunc2);
		ChokoLait::Paint(rendFunc, paintfunc2);
		auto m = Time::millis;
		VisSystem::uiMs = (uint)(m - lastMillis);
		if (dirty)
			VisSystem::renderMs = VisSystem::uiMs;
		lastMillis = m;
	}
	//*/
}