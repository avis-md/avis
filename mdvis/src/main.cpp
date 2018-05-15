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
#include "utils/spline.h"
#include "utils/solidify.h"

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
	VisSystem::Init();

	Gromacs::Read(IO::path + "/md.gro");
	ParGraphics::UpdateDrawLists();

	glEnable(GL_PROGRAM_POINT_SIZE);

	Display::Resize(800, 600, false);

	auto lastMillis = Time::millis;
	bool dirty = false;
	
	ChokoLait::mainCamera->object->transform.localPosition(Vec3(0, 0, -1));
	
	Vec3 pts[5]{ Vec3(0, 0, 0.2f), Vec3(0, 0.2f, 0), Vec3(0.1f, 0.3f, 1), Vec3(0.4f, 0.05f, 1), Vec3(0.8f, 0.6f, 0.5f) };
	Vec3 res[4 * 12 + 1];
	Spline::ToSpline(pts, 5, 12, res);
	splineMesh = Solidify::Do(res, 4 * 12 + 1, 0.01f, 12);

	Shader* proShad = new Shader(IO::GetText(IO::path + "/proV.txt"), IO::GetText(IO::path + "/proF.txt"));
	pMaterial proMat = std::make_shared<Material>(proShad);

	auto obj = SceneObject::New("123");
	Scene::active->AddObject(obj);
	obj->AddComponent<MeshFilter>()->mesh(splineMesh);
	obj->AddComponent<MeshRenderer>()->materials[0](proMat);

	glfwShowWindow(Display::window);

	while (ChokoLait::alive()) {
		ChokoLait::Update(updateFunc);
		//ChokoLait::mainCamera->object->transform.localPosition(Vec3(0, 0, -camz));
		//ChokoLait::mainCamera->object->transform.localPosition(-ParGraphics::rotCenter);

		dirty = Scene::dirty;
		//ChokoLait::Paint(nullptr, paintfunc2);
		ChokoLait::Paint(rendFunc, paintfunc);
		auto m = Time::millis;
		VisSystem::uiMs = (uint)(m - lastMillis);
		if (dirty)
			VisSystem::renderMs = VisSystem::uiMs;
		lastMillis = m;
	}
	//*/
}