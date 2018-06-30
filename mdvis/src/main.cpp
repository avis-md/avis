#include "ChokoLait.h"
//#define NPY_NO_DEPRECATED_API NPY_1_7_API_VERSION
//#include <numpy/arrayobject.h>

#include "ui/icons.h"
#include "ui/popups.h"
#include "ui/help.h"
#include "web/anweb.h"
#include "md/ParMenu.h"
#include "md/Protein.h"
#include "md/parloader.h"
#include "md/Gromacs.h"
#include "md/CDV.h"
#include "md/pdb.h"
#include "md/XYZ.h"
#include "md/mdvbin.h"
#include "vis/pargraphics.h"
#include "vis/system.h"
#include "vis/shadows.h"
#include "utils/effects.h"
#include "utils/ssh.h"
#include "mdchan.h"
#include "live/livesyncer.h"

bool __debug = false;

float camz = 10;
Vec3 center;
float rw = 0, rz = 0;
float repx, repy, repz;

Gromacs* gro;
Shader* shad, *shad2;
GLuint emptyVao;

bool drawMesh;

pMesh splineMesh;

float zoomFade;

void rendFunc() {
	ParGraphics::Rerender();
	if (!!Particles::particleSz && Shadows::show) {
		Shadows::UpdateBox();
		Mat4x4 _p = MVP::projection();
		MVP::Switch(true);
		MVP::Clear();
		Shadows::Rerender();
		MVP::Switch(true);
		MVP::Clear();
		MVP::Mul(_p);
	}
}

void updateFunc() {
	if (ParLoader::parDirty) {
		ParLoader::parDirty = false;
		Particles::UpdateBufs();
		Particles::GenTexBufs();
		Protein::Refresh();
		ParGraphics::UpdateDrawLists();
	}

	ParGraphics::Update();
	LiveSyncer::Update();

	AnWeb::Update();

	if (!UI::editingText && !AnWeb::drawFull && Input::KeyDown(Key_F)) {
		auto& o = ChokoLait::mainCamera->ortographic;
		o = !o;
		Scene::dirty = true;
	}
}

void paintfunc() {
	bool stealFocus = false;

	if (AnWeb::drawFull)
		AnWeb::Draw();
	else {
		ParMenu::Draw();
		if (!Particles::particleSz || LiveSyncer::activeRunner) {
			LiveSyncer::DrawSide();
		}
		else {
			AnWeb::DrawSide();
		}

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

	ParLoader::DrawOpenDialog();
	Popups::Draw();

	auto pos = Input::mousePos;

	ParGraphics::hlIds.clear();
	if (UI::_layerMax == 0 && !stealFocus && VisSystem::InMainWin(Input::mousePos)) {

		auto id = ChokoLait::mainCamera->GetIdAt((uint)Input::mousePos.x, (uint)Input::mousePos.y);
		if (!!id) {
			ParGraphics::hlIds.push_back(id);
			if (Input::mouse0State == 1) {
				if (Input::KeyHold(Key_LeftControl)) {
					if (Input::KeyHold(Key_LeftShift)) {
						auto f = std::find(ParGraphics::selIds.begin(), ParGraphics::selIds.end(), id);
						if (f == ParGraphics::selIds.end()) ParGraphics::selIds.push_back(id);
						else ParGraphics::selIds.erase(f);
					}
					else {
						ParGraphics::selIds.resize(1);
						ParGraphics::selIds[0] = id;
						std::cout << "asdf";
					}
				}
				else if (Input::dbclick)
					ParGraphics::rotCenter = Particles::particles_Pos[id];
			}

			id--;
			Engine::DrawQuad(Input::mousePos.x + 14, Input::mousePos.y + 2, 120, 60, white(0.8f, 0.1f));
			//UI::Label(Input::mousePos.x + 14, Input::mousePos.y + 2, 12, "Particle " + std::to_string(id), white());
			//UI::Label(Input::mousePos.x + 14, Input::mousePos.y + 17, 12, &Particles::particles_ResName[id * PAR_MAX_NAME_LEN], PAR_MAX_NAME_LEN, white());
			//UI::Label(Input::mousePos.x + 14, Input::mousePos.y + 32, 12, &Particles::particles_Name[id * PAR_MAX_NAME_LEN], PAR_MAX_NAME_LEN, white());
			//UI::Label(Input::mousePos.x + 14, Input::mousePos.y + 47, 12, std::to_string(Particles::particles_Pos[id]), font, white());

		}
		else {
			if ((Input::mouse0State == 1) && Input::KeyHold(Key_LeftControl)) {
				ParGraphics::selIds.clear();
			}
		}
	}

	HelpMenu::Draw();
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
	GLFWimage icon;
	byte chn;
	icon.pixels = Texture::LoadPixels(IO::path + "/res/icon.png", chn, (uint&)icon.width, (uint&)icon.height);
	if (icon.pixels) glfwSetWindowIcon(Display::window, 1, &icon);
	delete[](icon.pixels);

	SSH::Init();
	Icons::Init();
	VisSystem::Init();
	Particles::Init();
	ParLoader::Init();
	ParGraphics::Init();
	Protein::Init();
	PyReader::Init();
	AnWeb::Init();
	AnNode::Init();
	Effects::Init(0xffff);
	MdChan::Init();

	//ParLoader::Scan();

	AnBrowse::Scan();
	
	//pSceneObject lht = SceneObject::New(Vec3());
	//Scene::AddObject(lht);
	//auto l = lht->AddComponent<Light>();
	//ParGraphics::SetLight(l.get());

	ParImporter* imp = new ParImporter();
	imp->name = "Gromacs";
	imp->sig = "gro";
	imp->funcs.push_back(std::pair<std::vector<string>, ParImporter::loadsig>());
	imp->funcs.back().first.push_back(".gro");
	imp->funcs.back().second = Gromacs::Read;
	ParLoader::importers.push_back(imp);
	
	imp = new ParImporter();
	imp->name = "Protein DataBank";
	imp->sig = "pdb";
	imp->funcs.push_back(std::pair<std::vector<string>, ParImporter::loadsig>());
	imp->funcs.back().first.push_back(".pdb");
	imp->funcs.back().second = PDB::Read;
	ParLoader::importers.push_back(imp);
	
	imp = new ParImporter();
	imp->name = "XYZ coords";
	imp->sig = "xyz";
	imp->funcs.push_back(std::pair<std::vector<string>, ParImporter::loadsig>());
	imp->funcs.back().first.push_back(".xyz");
	imp->funcs.back().second = XYZ::Read;
	ParLoader::importers.push_back(imp);

	imp = new ParImporter();
	imp->name = "CDView";
	imp->sig = "cdv";
	imp->funcs.push_back(std::pair<std::vector<string>, ParImporter::loadsig>());
	imp->funcs.back().first.push_back(".cdv");
	imp->funcs.back().second = CDV::Read;
	ParLoader::importers.push_back(imp);

	imp = new ParImporter();
	imp->name = "binary";
	imp->sig = "bin";
	imp->funcs.push_back(std::pair<std::vector<string>, ParImporter::loadsig>());
	imp->funcs.back().first.push_back(".bin");
	imp->funcs.back().second = MDVBin::Read;
	ParLoader::importers.push_back(imp);

	//AnWeb::Load(IO::path + "/nodes/rdf.anl");
	AnWeb::nodes.push_back(new Node_Recolor_All());
	AnWeb::nodes.push_back(new Node_AddBond());
	//CDV::_Read(IO::path + "/ayuba/position000000.cdv", false);
	//CDV::_ReadTrj(IO::path + "/ayuba/position");
	//Gromacs::Read(IO::path + "/pbc.gro", false);
	//bool ok = Gromacs::ReadTrj(IO::path + "/pbc.trr");
	//MDVBin::Read(IO::path + "/ayuba2/000000.bin", false);
	//MDVBin::ReadTrj(IO::path + "/ayuba2/");
	//string ss = IO::path + "/pbc.gro";
	//ParLoader::droppedFiles.resize(1, ss);
	//new std::thread(ParLoader::DoOpen);
	//ParLoader::DoOpen();
	//ParLoader::droppedFiles[0] = IO::path + "/pbc.trr";
	//ParLoader::DoOpenAnim();
	//ParLoader::OnDropFile(1, &c[0]);

	//Protein::Refresh();
	//ParGraphics::UpdateDrawLists();

	LiveRunner* runner = new LiveRunner();
	runner->initNm = "Init";
	runner->loopNm = "Loop";
	runner->path = IO::path + "/bin/liverunners/lj256/win32/lj256.dll";
	runner->name = "LJ256";
	LiveSyncer::runners.push_back(runner);
	//LiveSyncer::Init(0);
	//LiveSyncer::Start();

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