#include "ChokoLait.h"

//#define MAKE_RES
//#define MAKE_LOCL
#define NOCATCH
#define DELAYLOAD
//#define AUTOSAVE
#define DEBUG_CG

#include "ui/ui_ext.h"
#include "ui/browse.h"
#include "ui/help.h"
#include "ui/icons.h"
#include "ui/localizer.h"
#include "ui/popups.h"
#include "web/anweb.h"
#include "web/cc/creader.h"
#include "web/ft/freader.h"
#include "md/parmenu.h"
#include "md/Protein.h"
#include "imp/parloader.h"
#include "imp/GenericSSV.h"
#include "imp/Gromacs.h"
#include "imp/pdb.h"
#include "imp/pdbx.h"
#include "imp/CDV.h"
#include "imp/lammps.h"
#include "imp/dlpoly.h"
#include "vis/changelog.h"
#include "vis/cubemarcher.h"
#include "vis/pargraphics.h"
#include "vis/system.h"
#include "vis/shadows.h"
#include "vis/renderer.h"
#include "vis/selection.h"
#include "vis/preferences.h"
#include "utils/effects.h"
#include "utils/ssh.h"
#include "live/livesyncer.h"
#include "ocl/raytracer.h"
#include "res/resdata.h"
#include "hdr.h"

Unloader unloader;

bool __debug = false;
float autoSaveTime = 10;
std::vector<std::string> main_openfiles;
std::vector<std::string> main_openimpsigs;
bool option_silent = false, nologo = false;
bool option_min = false;

#ifdef DEBUG_CG
long long rendMs;
#endif

void rendFunc() {
	auto& cm = ChokoLait::mainCameraObj->transform;
#ifdef DEBUG_CG
	const auto lms = milliseconds();
#endif
	ParGraphics::Rerender(cm.position(), cm.forward(), (float)Display::width, (float)Display::height);
#ifdef DEBUG_CG
	rendMs = milliseconds() - lms;
#endif
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
		if (ParLoader::impId == 0) {
			for (auto& a : GenericSSV::attrs) {
				Particles::AddAttr(true);
				Particles::attrNms[Particles::attrs.size()-1] = a.first;
				auto& p = Particles::attrs.back();
				p->Get(0).swap(a.second);
				p->Seek(0);
			}
			Particles::readonlyAttrCnt = GenericSSV::attrs.size();
			GenericSSV::attrs.clear();
		}
		Particles::UpdateColorTex();
		Particles::UpdateBufs();
		Particles::UpdateBBox();
		Protein::Refresh();
		Particles::GenTexBufs();
		ParGraphics::OnLoadConfig();
		ParGraphics::UpdateDrawLists();
		ParMenu::CalcH();
		VisSystem::lastSave = Time::time;

		if (!!main_openfiles.size()) {
			ParLoader::directLoad = option_silent;
			ParLoader::OnOpenFile(main_openfiles);
		}
	}

	if (!!Particles::particleSz && !ParLoader::busy) {
		Particles::Update();
#ifdef AUTOSAVE
		if ((autoSaveTime > 1) && (Time::time - VisSystem::lastSave > autoSaveTime)
			&& !ParLoader::busy && !AnWeb::executing && ChokoLait::foreground
			&& (VisRenderer::status != VisRenderer::STATUS::BUSY)) {
			VisSystem::lastSave = Time::time;
			VisSystem::Save(IO::path + ".recover");
			VisSystem::SetMsg("autosaved at t=" + std::to_string((int)Time::time) + "s");
		}
#endif
	}

	ParGraphics::Update();
	//LiveSyncer::Update();

	AnWeb::Update();

	if (!!Particles::particleSz && !ParLoader::busy && !UI::editingText && !AnWeb::drawFull) {
		if (Input::KeyDown(KEY::F)) {
			auto& o = ChokoLait::mainCamera->ortographic;
			o = !o;
			Scene::dirty = true;
		}
		if (Input::KeyDown(KEY::X) && Input::KeyHold(KEY::LeftShift)) {
			if (!RayTracer::resTex) {
				RayTracer::SetScene();
				//RayTracer::Render();
			}
			else {
				RayTracer::UnsetScene();
				Scene::dirty = true;
			}
		}
		if (Input::KeyDown(KEY::F5)) {
			VisRenderer::ToImage();
		}
	}Quat() * Vec3();
	if (RayTracer::resTex) {
		if (ParGraphics::tfboDirty) {
			Scene::dirty = true;
		}
		RayTracer::Update();
		ParGraphics::tfboDirty = true;
	}
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
	ArrayView::Draw();

	if (!AnWeb::drawFull)
		VisSystem::DrawTitle();

	ParLoader::DrawOpenDialog();

	ParGraphics::hlIds.clear();
	if (!UI::_layerMax && !stealFocus && VisSystem::InMainWin(Input::mousePos) && !ParGraphics::dragging) {

		auto id = ChokoLait::mainCamera->GetIdAt((uint)Input::mousePos.x, (uint)Input::mousePos.y);
		if (id > 0 && id <= Particles::particleSz) {
			ParGraphics::hlIds.push_back(id);
			if ((Input::mouse0State == MOUSE_UP) && (Input::mouseDownPos == Input::mousePos)) {
				if (Input::dbclick) {
					ParGraphics::rotCenter = Particles::poss[id - 1];
					Scene::dirty = true;
				}
				if (Input::KeyHold(KEY::LeftShift)) {
					auto f = std::find(Selection::atoms.begin(), Selection::atoms.end(), id - 1);
					if (f == Selection::atoms.end()) Selection::atoms.push_back(id - 1);
					else if (!Input::dbclick) Selection::atoms.erase(f);
					else goto nore;
					Selection::Recalc();
				nore:;
				}
				else {
					Selection::atoms.resize(1);
					Selection::atoms[0] = id - 1;
					Selection::Recalc();
				}
				//auto& rl = Particles::ress[id-1];
				//Particles::residueLists[rl.x].expanded = true;
				//Particles::residueLists[rl.x].residues[rl.y].expanded = true;
			}

			id--;
#if 1
			const float x0 = std::round(Input::mousePos.x + 16);
			const float y0 = std::round(Input::mousePos.y + 2);
			UI2::BackQuad(x0 - 2, y0, 120, 90);
			UI::Label(x0, y0, 12, "Atom ID: " + std::to_string(id), white());
			UI::Label(x0, y0 + 15, 12, "Residue: " + std::string(&Particles::resNames[id * PAR_MAX_NAME_LEN], PAR_MAX_NAME_LEN), white());
			UI::Label(x0, y0 + 30, 12, "Atom: " + std::string(&Particles::names[id * PAR_MAX_NAME_LEN], PAR_MAX_NAME_LEN), white());
			UI::Label(x0, y0 + 45, 12, "X: " + std::to_string(Particles::poss[id].x), white());
			UI::Label(x0, y0 + 60, 12, "Y: " + std::to_string(Particles::poss[id].y), white());
			UI::Label(x0, y0 + 75, 12, "Z: " + std::to_string(Particles::poss[id].z), white());
#endif
		}
		else {
			if ((Input::mouse0State == MOUSE_UP) && (Input::mouseDownPos == Input::mousePos)) {
				Selection::Clear();
			}
		}
	}

	VisRenderer::Draw();
	ParMenu::DrawSplash();
	ChangeLog::Draw();
	Browse::Draw();
	Preferences::Draw();
	HelpMenu::Draw();

	Popups::Draw();
	UI2::DrawTooltip();
	
#ifdef DEBUG_CG
	UI::Label(10, 10, 12, "Render: " + std::to_string(rendMs) + "ms", white(), true);
#endif
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
		Debug::suppress = 1;
		for (auto a = 1; a < argc; ++a) {
			if (argv[a][0] == '-') {
				if (argv[a][1] == '-') {
					argv[a] += 2;
#define ISS(str) (!strcmp(argv[a], #str))
#define ISC(c) (argv[a][0] == c)
					if ISS(debug)
						__debug = true;
					else if ISS(version) {
						std::cout << VERSIONSTRING << std::endl
							<< "hash: " << VisSystem::version_hash << std::endl
							<< "built on " << __DATE__ << std::endl;
						return 0;
					}
					else if ISS(help) {
						std::cout << "AViS\n" << VERSIONSTRING "\n"
							<< res::helpText << std::endl;
						return 0;
					}
					else if ISS(path) {
						IO::InitPath();
						std::cout << IO::path << std::endl;
						return 0;
					}
					else if ISS(nologo) {
						nologo = true;
					}
					else {
						std::cout << "Unknown switch " << argv[a] - 2 << ". Type --help for usage guide." << std::endl;
						return -1;
					}
				}
				else {
					argv[a] ++;
					if ISC('s')
						option_silent = true;
					else if ISC('v') {
						std::cout << VERSIONSTRING << std::endl
							<< "hash: " << VisSystem::version_hash << std::endl
							<< "built on " << __DATE__ << std::endl;
						if (argc == 2)
							return 0;
					}
					else if ISC('b') {
						if (argv[a][1] == '0') ParLoader::useConn = false;
						else if (argv[a][1] == '1') ParLoader::useConn = true;
						else {
							std::cout << "Value of -b must be either 0 or 1. Type --help for usage guide." << std::endl;
							return -2;
						}
					}
					else if ISC('f') {
						ParLoader::maxframes = TryParse(argv[a] + 1, -1);
					}
					else if ISC('m') {
						option_min = true;
						ParMenu::expanded = false;
						AnWeb::expanded = false;
					}
					else if ISC('i') {
						main_openimpsigs.push_back(argv[a]+1);
					}
					else {
						std::cout << "Unknown option " << argv[a] - 1 << ". Type --help for usage guide." << std::endl;
						return -2;
					}
#undef ISS
				}
			}
			else {
				main_openfiles.push_back(argv[a]);
			}
		}
		if (!nologo) {
			std::cout << "AViS " APPVERSION "\n" R"(Pua Kai, 2018
Thanks for trying out this program!
You can view a copy of all logs in Log.txt.
Raw compile output for each script is in __[]cache__/name_log.txt.
The hash for this program is )" << VisSystem::version_hash
			<< "\n(I may need this hash to fix bugs)"
			<< "\n----------------------------------------------" << std::endl;
		}
		if (__debug) Debug::suppress = 0;
		else if (!nologo) std::cout << "Starting in silent mode. You can enable all logs using the --debug switch." << std::endl;
		ChokoLait::Init(800, 800);

#ifdef MAKE_LOCL
		std::string path = __FILE__;
#ifdef PLATFORM_WIN
		std::replace(path.begin(), path.end(), '\\', '/');
#endif
		path = path.substr(0, path.find_last_of('/'));
		Localizer::MakeMap(path);
		return 0;
#else
		Preferences::Init();
		Localizer::Init("");
		//Localizer::Init(VisSystem::prefs["SYS_LOCALE"]);
		Preferences::Link("SDPI", &Display::dpiScl, &Display::OnDpiChange);
#endif
		//GLFWimage icon;
		//icon.pixels = Texture::LoadPixels(res::icon_png, res::icon_png_sz, (uint&)icon.width, (uint&)icon.height);
		//glfwSetWindowIcon(Display::window, 1, &icon);
		//delete[](icon.pixels);

#define INIT(nm, ...) Debug::Message("System", "Initializing " #nm); nm::Init(__VA_ARGS__)
#ifdef DELAYLOAD
#define LINIT(nm, ...) Debug::Message("System", "Skipping " #nm)
#else
#define LINIT INIT
#endif

		Random::Seed((uint)milliseconds());
		INIT(Font);
		INIT(UI);
		INIT(UI2);
		INIT(UI3);
		INIT(Browse);
		INIT(CReader);
		LINIT(PyReader);
		//INIT(FReader);
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
		INIT(AnBrowse);
		INIT(ChangeLog);
		ParMenu::LoadRecents();

		Preferences::Load();
		CReader::LoadReader();
		FReader::LoadReader();
		AnBrowse::Scan();
		//AnBrowse::Refresh();

		ParImporter imp = {};
		ParImporter::Func fnc = {};
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
#define SETTRJ(_ext, _fnc) \
		fnc = {}; \
		fnc.type = ParImporter::Func::FUNC_TYPE::TRAJ;\
		fnc.exts.push_back(#_ext);\
		fnc.trjFunc = _fnc;\
		imp.funcs.push_back(fnc);\
		ParLoader::exts.push_back("*" #_ext);
#define PUSHIMP ParLoader::importers.push_back(imp); \
		imp = {};

		NEWIMP("Generic SSV", ssv, .ssv, GenericSSV::Read)
		SETFRM(.ssv, GenericSSV::ReadFrm)
		PUSHIMP
		NEWIMP("Gromacs", gro, .gro, Gromacs::Read)
		SETFRM(.gro, Gromacs::ReadFrm)
		SETTRJ(.trr, Gromacs::ReadTrr)
		SETTRJ(.xtc, Gromacs::ReadXtc)
		PUSHIMP
		NEWIMP("Protein DataBank", pdb, .pdb, PDB::Read)
		SETFRM(.pdb, PDB::ReadFrm)
		PUSHIMP
		NEWIMP("Protein DataBank X", pdbx, .cif, PDBx::Read)
		PUSHIMP
		NEWIMP("CDView", cdv, .cdv, CDV::Read)
		SETFRM(.cdv, CDV::ReadFrame)
		PUSHIMP
		NEWIMP("Lammps", lmp, .atom, Lammps::Read)
		PUSHIMP
		NEWIMP("DLPoly", dlp, .000, DLPoly::Read)
		PUSHIMP

		if (!!main_openimpsigs.size()) {
			ParLoader::requestSig = main_openimpsigs[0];
		}
		if (!!main_openfiles.size()) {
			for (auto& f : main_openfiles) f = IO::FullPath(f);
			ParLoader::directLoad = option_silent;
			ParLoader::OnOpenFile(main_openfiles);
			main_openfiles.erase(main_openfiles.begin());
		}

		if (option_min)
			Display::Resize(500, 500, false);
		else
			Display::Resize(1024, 600, false);

		auto lastMillis = Time::millis;

		ChokoLait::mainCameraObj->transform.localPosition(Vec3(0, 0, -1));
		ChokoLait::mainCamera->quality = 1;
		ChokoLait::mainCamera->useGBuffer2 = true;
		ChokoLait::mainCamera->quality2 = 0.5f;

		glfwShowWindow(Display::window);

		while (ChokoLait::alive()) {
			Engine::stateLock.lock();
			if (!Display::width || !Display::height)
				glfwPollEvents();
			else {
				UI2::PreLoop();
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
			Engine::WaitForLockValue();
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
