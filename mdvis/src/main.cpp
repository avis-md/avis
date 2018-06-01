#include "ChokoLait.h"
#include "md/Particles.h"
#include "md/Gromacs.h"
//#define NPY_NO_DEPRECATED_API NPY_1_7_API_VERSION
//#include <numpy/arrayobject.h>

#include "ui/icons.h"
#include "ui/popups.h"
#include "py/PyWeb.h"
#include "md/ParMenu.h"
#include "md/Protein.h"
#include "vis/pargraphics.h"
#include "vis/system.h"
#include "utils/effects.h"

bool __debug = false;

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

float zoomFade;

void rendFunc() {
	ParGraphics::Rerender();
}

void updateFunc() {
	ParGraphics::Update();

	PyWeb::Update();

	if (Input::KeyDown(Key_F)) {
		auto& o = ChokoLait::mainCamera->ortographic;
		o = !o;
		Scene::dirty = true;
	}
}

void paintfunc() {
	bool stealFocus = false;

	if (PyWeb::drawFull)
		PyWeb::Draw();
	else {
		ParMenu::Draw();
		PyWeb::DrawSide();

		if (ParGraphics::zoomFade > 0) {
			auto zf = min(ParGraphics::zoomFade * 2, 1.0f);
			Engine::DrawQuad(Display::width * 0.5f - 150.0f, Display::height - 100.0f, 300, 20, white(zf * 0.9f, 0.15f));
			UI::Texture(Display::width * 0.5f - 150.0f, Display::height - 98.0f, 16, 16, Icons::zoomOut, white(zf));
			UI::Texture(Display::width * 0.5f + 134.0f, Display::height - 98.0f, 16, 16, Icons::zoomIn, white(zf));
			Engine::DrawQuad(Display::width * 0.5f - 130.0f, Display::height - 91.0f, 260, 2, white(zf, 0.8f));
			Engine::DrawQuad(Display::width * 0.5f - 133.0f + 260 * InverseLerp(-6.0f, 2.0f, ParGraphics::rotScale), Display::height - 98.0f, 6, 16, white(zf));
			ParGraphics::zoomFade -= Time::delta;
		}
	}
	VisSystem::DrawBar();

	Popups::Draw();

	auto pos = Input::mousePos;

	ParGraphics::hlIds.clear();
	if (UI::_layerMax == 0 && !stealFocus && VisSystem::InMainWin(Input::mousePos)) {

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
	for (auto a = 0; a < argc; a++) {
		if (argv[a][0] == '-') {
			if (!strcmp(argv[a] + 1, "debug"))
				__debug = true;
		}
	}

	if (!__debug) Debug::suppress = 1;

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

	Effects::Init(0xffff);

	pSceneObject lht = SceneObject::New(Vec3());
	Scene::AddObject(lht);
	auto l = lht->AddComponent<Light>();
	ParGraphics::SetLight(l.get());

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