#include "ChokoLait.h"
#include "md/Particles.h"
#include "md/Gromacs.h"
//#define NPY_NO_DEPRECATED_API NPY_1_7_API_VERSION
//#include <numpy/arrayobject.h>

#include "ui/icons.h"
#include "py/PyWeb.h"
#include "md/ParMenu.h"
#include "md/Protein.h"
#include "vis/pargraphics.h"
#include "vis/system.h"

float camz = 10;
Vec3 center;
float rw = 0, rz = 0;
float repx, repy, repz;

Gromacs* gro;
Shader* shad, *shad2;
Font* font;
GLuint emptyVao;

bool drawMesh;

pMesh splineMesh;

void rendFunc() {
	ParGraphics::Rerender();
}

void updateFunc() {
	ParGraphics::Update();

	PyWeb::Update();
}

void paintfunc() {
	if (PyWeb::drawFull)
		PyWeb::Draw();
	else {
		ParMenu::Draw();
		PyWeb::DrawSide();
	}
	VisSystem::DrawBar();

	auto pos = Input::mousePos;

	ParGraphics::hlIds.clear();
	if (VisSystem::InMainWin(Input::mousePos)) {
		auto id = ChokoLait::mainCamera->GetIdAt((uint)Input::mousePos.x, (uint)Input::mousePos.y);
		if (!!id) {
			ParGraphics::hlIds.push_back(id);
			id--;
			Engine::DrawQuad(Input::mousePos.x + 14, Input::mousePos.y + 2, 120, 60, white(0.8f, 0.1f));
			UI::Label(Input::mousePos.x + 14, Input::mousePos.y + 2, 12, "Particle " + std::to_string(id), font, white());
			UI::Label(Input::mousePos.x + 14, Input::mousePos.y + 17, 12, &Particles::particles_ResName[id * PAR_MAX_NAME_LEN], PAR_MAX_NAME_LEN, font, white());
			UI::Label(Input::mousePos.x + 14, Input::mousePos.y + 32, 12, &Particles::particles_Name[id * PAR_MAX_NAME_LEN], PAR_MAX_NAME_LEN, font, white());
		}
	}
}

int main(int argc, char **argv)
{
	ChokoLait::Init(800, 800);
	font = new Font(IO::path + "/arimo.ttf", ALIGN_TOPLEFT);
	
	VisSystem::font = font;

	Icons::Init();
	VisSystem::Init();
	Particles::Init();
	ParGraphics::Init();
	Protein::Init();
	
	PyReader::Init();
	PyNode::Init();
	PyNode::font = font;

	PyBrowse::Scan();
	
	PyWeb::Init();
	ParMenu::font = font;

	Gromacs::Read(IO::path + "/pbc.gro", false);
	bool ok = Gromacs::ReadTrj(IO::path + "/pbc.trr");
	Protein::Refresh();
	ParGraphics::UpdateDrawLists();
	
	glEnable(GL_PROGRAM_POINT_SIZE);

	Display::Resize(800, 600, false);

	auto lastMillis = Time::millis;
	bool dirty = false;
	
	ChokoLait::mainCamera->object->transform.localPosition(Vec3(0, 0, -1));
	ChokoLait::mainCamera->quality = 1;
	ChokoLait::mainCamera->quality2 = 1;

	glfwShowWindow(Display::window);

	while (ChokoLait::alive()) {
		if (!Display::width || !Display::height)
			glfwPollEvents();
		else {
			ChokoLait::Update(updateFunc);
			dirty = Scene::dirty;
			ChokoLait::Paint(rendFunc, paintfunc);
			auto m = Time::millis;
			VisSystem::uiMs = (uint)(m - lastMillis);
			if (dirty)
				VisSystem::renderMs = VisSystem::uiMs;
			lastMillis = m;
		}
	}
	//*/
}