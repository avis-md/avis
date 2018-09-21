#include "ChokoLait.h"

//#define MAKE_RES
//#define MAKE_LOCL
#define NOCATCH

#include "ui/ui_ext.h"
#include "ui/localizer.h"
#include "ui/icons.h"
#include "ui/popups.h"
#include "ui/help.h"
#include "web/anweb.h"
#include "md/ParMenu.h"
#include "md/Protein.h"
#include "md/parloader.h"
#include "md/Gromacs.h"
//#include "md/CDV.h"
//#include "md/pdb.h"
//#include "md/XYZ.h"
//#include "md/mdvbin.h"
//#include "md/lammps.h"
//#include "md/dlpoly.h"
#include "vis/cubemarcher.h"
#include "vis/pargraphics.h"
#include "vis/system.h"
#include "vis/shadows.h"
#include "vis/renderer.h"
#include "utils/effects.h"
#include "utils/ssh.h"
#include "live/livesyncer.h"
#include "ocl/raytracer.h"
#include "res/resdata.h"

bool __debug = false;
float autoSaveTime = 30;

CubeMarcher* cm;
Mat4x4 _mv, _p;

void rendFunc() {
	auto& cm = ChokoLait::mainCamera->object->transform;
	ParGraphics::Rerender(cm.position(), cm.forward(), (float)Display::width, (float)Display::height);
#define bb Particles::boundingBox
	UI3::Cube(bb[0], bb[1], bb[2], bb[3], bb[4], bb[5], blue());
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
	if (RayTracer::resTex) RayTracer::Render();
	_mv = MVP::modelview();
	_p = MVP::projection();
	AnWeb::OnSceneUpdate();
}

void updateFunc() {
	if ((autoSaveTime > 1) && (Time::time - VisSystem::lastSave > autoSaveTime)) {
		VisSystem::lastSave = Time::time;
		VisSystem::Save(IO::path + ".recover");
	}

	if (ParLoader::parDirty) {
		ParLoader::parDirty = false;
		Particles::UpdateBufs();
		if (Protein::Refresh()) {
			Particles::GenTexBufs();
			ParGraphics::UpdateDrawLists();
		}
		VisSystem::lastSave = Time::time;
	}

	ParGraphics::Update();
	LiveSyncer::Update();

	AnWeb::Update();

	if (!!Particles::particleSz && !UI::editingText && !AnWeb::drawFull) {
		if (Input::KeyDown(Key_F)) {
			auto& o = ChokoLait::mainCamera->ortographic;
			o = !o;
			Scene::dirty = true;
		}
		if (Input::KeyDown(Key_X) && Input::KeyHold(Key_LeftShift)) {
			if (!RayTracer::resTex) {
				RayTracer::SetScene();
				//RayTracer::Render();
			}
			else {
				RayTracer::Clear();
			}
		}
		if (Input::KeyDown(Key_F5)) {
			VisRenderer::ToImage();
		}
	}
	if (RayTracer::resTex) {
		
	}
}

void paintfunc() {
	bool stealFocus = false;

	if (!Particles::particleSz) {
		//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		ParMenu::DrawStart();
	}

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
			UI::Quad(Display::width * 0.5f - 150.0f, Display::height - 100.0f, 300, 20, white(zf * 0.9f, 0.15f));
			UI::Texture(Display::width * 0.5f - 150.0f, Display::height - 98.0f, 16, 16, Icons::zoomOut, white(zf));
			UI::Texture(Display::width * 0.5f + 134.0f, Display::height - 98.0f, 16, 16, Icons::zoomIn, white(zf));
			UI::Quad(Display::width * 0.5f - 130.0f, Display::height - 91.0f, 260, 2, white(zf, 0.8f));
			UI::Quad(Display::width * 0.5f - 133.0f + 260 * InverseLerp(-6.0f, 2.0f, ParGraphics::rotScale), Display::height - 98.0f, 6, 16, white(zf));
			ParGraphics::zoomFade -= Time::delta;
		}
	}
	VisSystem::DrawBar();

	ErrorView::Draw();

	if (!AnWeb::drawFull)
		VisSystem::DrawTitle();

	ParLoader::DrawOpenDialog();
	Popups::Draw();

	auto pos = Input::mousePos;

	ParGraphics::hlIds.clear();
	if (!UI::_layerMax && !stealFocus && VisSystem::InMainWin(Input::mousePos) && !ParGraphics::dragging) {

		auto id = ChokoLait::mainCamera->GetIdAt((uint)Input::mousePos.x, (uint)Input::mousePos.y);
		if (!!id) {
			//std::cout << id << std::to_string(Input::mousePos) << std::endl;
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
					}
				}
				else if (Input::dbclick)
					ParGraphics::rotCenter = Particles::particles_Pos[id - 1];
			}

			id--;
			UI::Quad(Input::mousePos.x + 14, Input::mousePos.y + 2, 120, 60, white(0.8f, 0.1f));
			UI::Label(Input::mousePos.x + 14, Input::mousePos.y + 2, 12, "Particle " + std::to_string(id), white());
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

	VisRenderer::Draw();
	if (ParMenu::showSplash) ParMenu::DrawSplash();
	HelpMenu::Draw();
}

#ifdef MAKE_RES
#include "makeres.h"
int main(int argc, char **argv) {
	MakeRes::Do();
	std::getline(std::cin, *(new std::string()));
	return 0;
#else
int main(int argc, char **argv) {
#endif

#ifndef NOCATCH
	try {
#endif
		Time::startMillis = milliseconds();

		std::vector<std::string> fls;
		bool _s = false, _x = false;
		std::string _xs = "";
		for (auto a = 1; a < argc; a++) {
			if (argv[a][0] == '-') {
				if (argv[a][1] == '-') {
					argv[a] += 2;
#define ISS(str) (!strcmp(argv[a], #str))
					if ISS(debug)
						__debug = true;
					else if ISS(help) {
						std::cout << res::helpText;
						return 0;
					}
					else {
						std::cout << "Unknown switch " << argv[a] - 2 << ". Type --help for usage guide.";
						return -1;
					}
				}
				else {
					argv[a] ++;
					if ISS(s)
						_s = true;
					else {
						std::cout << "Unknown option " << argv[a] - 1 << ". Type --help for usage guide.";
						return -2;
					}
#undef ISS
				}
			}
			else {
				fls.push_back(argv[a]);
			}
		}
		if (!__debug) Debug::suppress = 1;
		ChokoLait::Init(800, 800);

#ifdef MAKE_LOCL
		Localizer::MakeMap(
#ifdef PLATFORM_WIN
			"../mdvis/src");
#else
			"../src");
#endif
		return 0;
#else
		VisSystem::InitEnv();
		Localizer::Init(VisSystem::prefs["SYS_LOCALE"]);
#endif
		//GLFWimage icon;
		//icon.pixels = Texture::LoadPixels(res::icon_png, res::icon_png_sz, (uint&)icon.width, (uint&)icon.height);
		//glfwSetWindowIcon(Display::window, 1, &icon);
		//delete[](icon.pixels);

#define INITS(nm) nm::Init()
#define INIT(nm) Debug::Message("System", "Initializing " #nm); INITS(nm)
		INIT(Font);
		INIT(UI);
		INIT(UI2);
		INIT(UI3);
		INIT(CReader);
		INIT(PyReader);
		INIT(FReader);
		//INIT(RayTracer);
		INIT(Color);
		INIT(Icons);
		INIT(CubeMarcher);
		INIT(VisSystem);
		INIT(Particles);
		INIT(ParMenu);
		INIT(ParLoader);
		INIT(ParGraphics);
		INIT(Protein);
		INIT(Shadows);
		INIT(AnWeb);
		INIT(AnNode);
		Effects::Init(0xffff);
		ParMenu::LoadRecents();
		//SSH::Init();

		AnBrowse::Scan();

		ParImporter* imp = new ParImporter();
		imp->name = "Gromacs";
		imp->sig = "gro";
		imp->funcs.push_back(std::pair<std::vector<std::string>, ParImporter::loadsig>());
		imp->funcs.back().first.push_back(".gro");
		imp->funcs.back().second = Gromacs::Read;
		imp->trjFuncs.push_back(std::pair<std::vector<std::string>, ParImporter::loadtrjsig>());
		imp->trjFuncs.back().first.push_back(".trr");
		imp->trjFuncs.back().second = Gromacs::ReadTrj;
		ParLoader::importers.push_back(imp);

		ParLoader::exts = std::vector<std::string>({ "*.gro", "*.trr" });

#define NEWIMP(_nm, _sig, _ext, _fnc) \
		imp = new ParImporter(); \
		imp->name = #_nm; \
		imp->sig = #_sig; \
		imp->funcs.push_back(std::pair<std::vector<std::string>, ParImporter::loadsig>()); \
		imp->funcs.back().first.push_back(#_ext); \
		imp->funcs.back().second = _fnc::Read; \
		ParLoader::importers.push_back(imp); \
		ParLoader::exts.push_back(#_ext);

		//NEWIMP(Protein DataBank, pdb, .pdb, PDB);
		//NEWIMP(XYZ coords, xyz, .xyz, XYZ);
		//NEWIMP(CDView, cdv, .cdv, CDV);
		//NEWIMP(Binary, bin, .bin, MDVBin);
		//NEWIMP(Lammps, lmp, .atom, Lammps);
		//NEWIMP(DLPoly, dlp, .000, DLPoly);

		if (fls.size()) {
			ParLoader::directLoad = _s;
			ParLoader::OnOpenFile(fls);
		}

		LiveRunner* runner = new LiveRunner();
		runner->initNm = "Init";
		runner->loopNm = "Loop";
		runner->path = IO::path + "bin/liverunners/lj256/win32/lj256.dll";
		runner->name = "LJ256";
		LiveSyncer::runners.push_back(runner);

		Display::Resize(800, 600, false);

		auto lastMillis = Time::millis;
		bool dirty = false;

		ChokoLait::mainCamera->object->transform.localPosition(Vec3(0, 0, -1));
		ChokoLait::mainCamera->quality = 1;
		ChokoLait::mainCamera->quality2 = 1;

		glfwShowWindow(Display::window);
		
		AnWeb::Load(IO::path + "nodes/test.web");

		VisSystem::Save(IO::path + "test.xml");

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
		glfwDestroyWindow(Display::window);
		//*/
#ifndef NOCATCH
	}
	catch (...) {
		std::cout << "Something fatal happened!!\nPress enter to exit..." << std::endl;
		std::string s;
		std::getline(std::cin, s);
		return -1;
	}
#endif
}
