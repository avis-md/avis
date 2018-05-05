#include "ChokoLait.h"
#include "md/Particles.h"
#include "md/Gromacs.h"
//#define NPY_NO_DEPRECATED_API NPY_1_7_API_VERSION
//#include <numpy/arrayobject.h>

#include "ui/icons.h"
#include "py/PyWeb.h"
#include "md/ParMenu.h"
#include "vis/pargraphics.h"

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
	
	ParGraphics::Rerender();
}

void updateFunc() {
	float camz0 = camz;
	float rz0 = rz;
	float rw0 = rw;
	Vec3 center0 = center;
	if (Input::KeyHold(Key_UpArrow)) camz -= 10 * Time::delta;
	else if (Input::KeyHold(Key_DownArrow)) camz += 10 * Time::delta;
	camz = max(camz, 0.0f);
	if (Input::KeyHold(Key_W)) rw -= 100 * Time::delta;
	else if (Input::KeyHold(Key_S)) rw += 100 * Time::delta;
	//camz = Clamp<float>(camz, 0.5f, 10);
	if (Input::KeyHold(Key_A)) rz -= 100 * Time::delta;
	else if (Input::KeyHold(Key_D)) rz += 100 * Time::delta;
	//camz = Clamp<float>(camz, 0.5f, 10);
	if (Input::KeyHold(Key_J)) center.x -= 1 * Time::delta;
	else if (Input::KeyHold(Key_L)) center.x += 1 * Time::delta;
	if (Input::KeyHold(Key_I)) center.y -= 1 * Time::delta;
	else if (Input::KeyHold(Key_K)) center.y += 1 * Time::delta;
	if (Input::KeyHold(Key_U)) center.z -= 1 * Time::delta;
	else if (Input::KeyHold(Key_O)) center.z += 1 * Time::delta;

	if (camz0 != camz || rz0 != rz || rw0 != rw || center0 != center) Scene::dirty = true;

}

void paintfunc2() {
	PyWeb::Update();
	if (PyWeb::drawFull)
		PyWeb::Draw();
	else {
		ParMenu::Draw();
		PyWeb::DrawSide();
	}

	ParGraphics::hlIds.clear();
	if (!PyWeb::drawFull && (Input::mousePos.x > ParMenu::expandPos + 16) && (Input::mousePos.x < Display::width - PyWeb::expandPos)) {
		auto id = ChokoLait::mainCamera->GetIdAt((uint)Input::mousePos.x, (uint)Input::mousePos.y);
		if (id) {
			ParGraphics::hlIds.push_back(id);
			auto str = "Particle " + std::to_string(id-1) + "\n" + "Residue Name" + "\n" + Particles::particles_Name[id];
			Engine::DrawQuad(Input::mousePos.x + 14, Input::mousePos.y + 2, 120, 60, white(0.8f, 0.1f));
			UI::Label(Input::mousePos.x + 14, Input::mousePos.y + 2, 12, str, font, white());
		}
	}
}

int main(int argc, char **argv)
{
	ChokoLait::Init(800, 800);
	bg = new Background(IO::path + "/refl.hdr");
	font = new Font(IO::path + "/arimo.ttf", ALIGN_TOPLEFT);
	
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

	while (ChokoLait::alive()) {
		ChokoLait::Update(updateFunc);
		//ChokoLait::mainCamera->object->transform.localPosition(Vec3(0, 0, -camz));

		//ChokoLait::Paint(nullptr, paintfunc2);
		ChokoLait::Paint(rendFunc, paintfunc2);
	}
	//*/
}