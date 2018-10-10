#include "ChokoLait.h"

//#define MAKE_RES
//#define MAKE_LOCL
#define NOCATCH
#define DELAYLOAD

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
#include "md/pdb.h"
#include "md/CDV.h"
//#include "md/XYZ.h"
//#include "md/mdvbin.h"
#include "md/lammps.h"
#include "md/dlpoly.h"
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
float autoSaveTime = 10;

void rendFunc() {
	auto& cm = ChokoLait::mainCamera->object->transform;
	ParGraphics::Rerender(cm.position(), cm.forward(), static_cast<float>(Display::width), static_cast<float>(Display::height));
#define bb(i) static_cast<float>(Particles::boundingBox[i])
	//UI3::Cube(bb(0), bb(1), bb(2), bb(3), bb(4), bb(5), black());
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
	AnWeb::OnSceneUpdate();
}

void updateFunc() {
	if (ParLoader::parDirty) {
		ParLoader::parDirty = false;
		Particles::UpdateBufs();
		if (Protein::Refresh()) {
			Particles::GenTexBufs();
			ParGraphics::UpdateDrawLists();
		}
		ParMenu::CalcH();
		VisSystem::lastSave = Time::time;
	}

	if (!!Particles::particleSz) {
		Particles::Update();
		if ((autoSaveTime > 1) && (Time::time - VisSystem::lastSave > autoSaveTime) && !ParLoader::busy) {
			VisSystem::lastSave = Time::time;
			VisSystem::Save(IO::path + ".recover");
			VisSystem::SetMsg("autosaved at t=" + std::to_string((int)Time::time) + "s");
		}
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
		RayTracer::Refine();
	}


	//if (Input::KeyDown(Key_X) && Input::KeyHold(Key_LeftShift))
	//	if (!RayTracer::resTex)
	//		RayTracer::SetScene();
}

void paintfunc() {
	bool stealFocus = false;
	UI2::sepw = 0.5f;

	if (!Particles::particleSz) {
		ParMenu::DrawStart();
	}

	if (AnWeb::drawFull) {
		UI2::sepw = 0.33f;
		AnWeb::Draw();
		UI2::sepw = 0.5f;
	}
	else {
		ParMenu::Draw();
		UI2::sepw = 0.33f;
		AnWeb::DrawSide();
		UI2::sepw = 0.5f;

		ParGraphics::DrawOverlay();
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
			UI::Label(Input::mousePos.x + 14, Input::mousePos.y + 17, 12, &Particles::particles_ResName[id * PAR_MAX_NAME_LEN], PAR_MAX_NAME_LEN, white());
			UI::Label(Input::mousePos.x + 14, Input::mousePos.y + 32, 12, &Particles::particles_Name[id * PAR_MAX_NAME_LEN], PAR_MAX_NAME_LEN, white());
			UI::Label(Input::mousePos.x + 14, Input::mousePos.y + 47, 12, std::to_string(Particles::particles_Pos[id]), white());

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

	UI::Quad(0, 0, Display::width, Display::height, RayTracer::resTex);
}

#ifdef MAKE_RES
#include "makeres.h"
int main(int argc, char **argv) {
	IO::InitPath();
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
					else if ISS(version) {
						std::cout << VERSIONSTRING << std::endl
							<< "hash: " << VisSystem::version_hash << std::endl;
						return 0;
					}
					else if ISS(help) {
						std::cout << "MDVIS\n" << VERSIONSTRING "\n"
							<< res::helpText << std::endl;
						return 0;
					}
					else if ISS(path) {
						Debug::suppress = 1;
						IO::InitPath();
						std::cout << IO::path << std::endl;
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
					else if ISS(v) {
						std::cout << VERSIONSTRING << std::endl
							<< "hash: " << VisSystem::version_hash << std::endl;
						if (argc == 2)
							return 0;
					}
					else {
						std::cout << "Unknown option " << argv[a] - 1 << ". Type --help for usage guide." << std::endl;
						return -2;
					}
#undef ISS
				}
			}
			else {
				fls.push_back(argv[a]);
			}
		}
		std::cout << R"(Thanks for trying out this program! :)
You can view a copy of all logs in Log.txt.
Raw compile output for each script is in __[]cache__/name_log.txt.
The hash for this program is )" << VisSystem::version_hash
		<< "\n(I may need this hash to fix bugs)"
		<< "\n----------------------------------------------" << std::endl;
		if (!__debug) {
			Debug::suppress = 1;
			//
			std::cout << "Starting in silent mode. You can enable all logs using the --debug switch." << std::endl;
		}
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

#define INIT(nm, ...) Debug::Message("System", "Initializing " #nm); nm::Init(__VA_ARGS__)
#ifdef DELAYLOAD
#define LINIT(nm, ...) Debug::Message("System", "Skipping " #nm);
#else
#define LINIT INIT
#endif
		IO::MakeDirectory(IO::path + "tmp/");

		INIT(Font);
		INIT(UI);
		INIT(UI2);
		INIT(UI3);
		INIT(CReader);
		LINIT(PyReader);
		INIT(FReader);
		LINIT(RayTracer);
		INIT(Color);
		INIT(Icons);
		LINIT(CubeMarcher);
		INIT(VisSystem);
		INIT(Particles);
		INIT(ParMenu);
		INIT(ParLoader);
		INIT(ParGraphics);
		INIT(Protein);
		LINIT(Shadows);
		INIT(AnWeb);
		INIT(AnNode);
		INIT(Effects, 0xffff);
		INIT(SSH);
		ParMenu::LoadRecents();

		AnBrowse::Scan();

		ParImporter imp = ParImporter();
		imp.name = "Gromacs";
		imp.sig = "gro";
		ParImporter::Func fnc = {};
		fnc.type = ParImporter::Func::FUNC_TYPE::CONFIG;
		fnc.exts.push_back(".gro");
		fnc.func = Gromacs::Read;
		imp.funcs.push_back(fnc);
		fnc = {};
		fnc.type = ParImporter::Func::FUNC_TYPE::TRAJ;
		fnc.exts.push_back(".trr");
		fnc.trjFunc = Gromacs::ReadTrj;
		imp.funcs.push_back(fnc);
		ParLoader::importers.push_back(imp);

		ParLoader::exts = std::vector<std::string>({ "*.gro", "*.trr" });

#define NEWIMP(_nm, _sig, _ext, _fnc) \
		imp.name = _nm; \
		imp.sig = #_sig; \
		fnc = {}; \
		fnc.type = ParImporter::Func::FUNC_TYPE::CONFIG;\
		fnc.exts.push_back(#_ext);\
		fnc.func = _fnc;\
		imp.funcs.push_back(fnc);\
		ParLoader::exts.push_back("*" #_ext);
#define SETFRM(_ext, _fnc) \
		fnc = {}; \
		fnc.type = ParImporter::Func::FUNC_TYPE::FRAME;\
		fnc.exts.push_back(#_ext);\
		fnc.frmFunc = _fnc;\
		imp.funcs.push_back(fnc);\
		ParLoader::exts.push_back("*" #_ext);
#define PUSHIMP ParLoader::importers.push_back(imp); \
		imp = {};

		NEWIMP("Protein DataBank", pdb, .pdb, PDB::Read);
		PUSHIMP
		//NEWIMP(XYZ coords, xyz, .xyz, XYZ);
		NEWIMP("CDView", cdv, .cdv, CDV::Read)
		SETFRM(.cdv, CDV::ReadFrame)
		PUSHIMP
		//NEWIMP(Binary, bin, .bin, MDVBin);
		NEWIMP("Lammps", lmp, .atom, Lammps::Read)
		PUSHIMP
		NEWIMP("DLPoly", dlp, .000, DLPoly::Read);
		PUSHIMP

		if (fls.size()) {
			ParLoader::directLoad = _s;
			ParLoader::OnOpenFile(fls);
		}

		Display::Resize(1024, 600, false);

		auto lastMillis = Time::millis;

		ChokoLait::mainCamera->object->transform.localPosition(Vec3(0, 0, -1));
		ChokoLait::mainCamera->quality = 1;
		ChokoLait::mainCamera->quality2 = 1;

		glfwShowWindow(Display::window);

		while (ChokoLait::alive()) {
			Engine::stateLock.lock();
			if (!Display::width || !Display::height)
				glfwPollEvents();
			else {
				ChokoLait::Update(updateFunc);
				bool dirty = Scene::dirty;
				ChokoLait::Paint(rendFunc, paintfunc);
				auto m = Time::millis;
				VisSystem::uiMs = (uint)(m - lastMillis);
				if (dirty)
					VisSystem::renderMs = VisSystem::uiMs;
				lastMillis = m;
			}
			Engine::stateLock.unlock();
			while (Engine::stateLockId > 0){}
		}
		glfwDestroyWindow(Display::window);
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
