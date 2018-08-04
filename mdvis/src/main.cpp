#include "ChokoLait.h"

//#define MAKE_RES
//#define MAKE_LOCL

#include "ui/localizer.h"
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

CubeMarcher* cm;
Mat4x4 _mv, _p;

void rendFunc() {
	auto& cm = ChokoLait::mainCamera->object->transform;
	ParGraphics::Rerender(cm.position(), cm.forward(), Display::width, Display::height);
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
	if (ParLoader::parDirty) {
		ParLoader::parDirty = false;
		Particles::UpdateBufs();
		if (Protein::Refresh()) {
			Particles::GenTexBufs();
			ParGraphics::UpdateDrawLists();
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
		if (Scene::dirty) RayTracer::expDirty = true;
		else Scene::dirty = true;
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
			Engine::DrawQuad(Display::width * 0.5f - 150.0f, Display::height - 100.0f, 300, 20, white(zf * 0.9f, 0.15f));
			UI::Texture(Display::width * 0.5f - 150.0f, Display::height - 98.0f, 16, 16, Icons::zoomOut, white(zf));
			UI::Texture(Display::width * 0.5f + 134.0f, Display::height - 98.0f, 16, 16, Icons::zoomIn, white(zf));
			Engine::DrawQuad(Display::width * 0.5f - 130.0f, Display::height - 91.0f, 260, 2, white(zf, 0.8f));
			Engine::DrawQuad(Display::width * 0.5f - 133.0f + 260 * InverseLerp(-6.0f, 2.0f, ParGraphics::rotScale), Display::height - 98.0f, 6, 16, white(zf));
			ParGraphics::zoomFade -= Time::delta;
		}
	}
	VisSystem::DrawBar();

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
			Engine::DrawQuad(Input::mousePos.x + 14, Input::mousePos.y + 2, 120, 60, white(0.8f, 0.1f));
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
	std::getline(std::cin, *(new string()));
	return 0;
#else
int main(int argc, char **argv) {
#endif
	try {
		std::vector<string> fls;
		bool _s = false, _x = false;
		string _xs = "";
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

		CReader::Init();
		PyReader::Init();
		RayTracer::Init();
		Color::Init();
		Icons::Init();
		SSH::Init();
		CubeMarcher::Init();
		VisSystem::Init();
		Particles::Init();
		ParMenu::Init();
		ParLoader::Init();
		ParGraphics::Init();
		Protein::Init();
		AnWeb::Init();
		AnNode::Init();
		Effects::Init(0xffff);
		ParMenu::LoadRecents();

		AnBrowse::Scan();

		//cm = new CubeMarcher(0);

		ParImporter* imp = new ParImporter();
		imp->name = "Gromacs";
		imp->sig = "gro";
		imp->funcs.push_back(std::pair<std::vector<string>, ParImporter::loadsig>());
		imp->funcs.back().first.push_back(".gro");
		imp->funcs.back().second = Gromacs::Read;
		imp->trjFuncs.push_back(std::pair<std::vector<std::string>, ParImporter::loadtrjsig>());
		imp->trjFuncs.back().first.push_back(".trr");
		imp->trjFuncs.back().second = Gromacs::ReadTrj;
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

		imp = new ParImporter();
		imp->name = "lammps";
		imp->sig = "lmp";
		imp->funcs.push_back(std::pair<std::vector<string>, ParImporter::loadsig>());
		imp->funcs.back().first.push_back(".atom");
		imp->funcs.back().second = Lammps::Read;
		ParLoader::importers.push_back(imp);

		imp = new ParImporter();
		imp->name = "DL Poly";
		imp->sig = "dlp";
		imp->funcs.push_back(std::pair<std::vector<string>, ParImporter::loadsig>());
		imp->funcs.back().first.push_back(".000");
		imp->funcs.back().second = DLPoly::Read;
		ParLoader::importers.push_back(imp);

		ParLoader::exts = std::vector<string>({ "*.gro", "*.trr", "*.pdb", "*.xyz", "*.cdv", "*.bin", "*.atom", "*.000" });
		if (fls.size()) {
			ParLoader::directLoad = _s;
			ParLoader::OnOpenFile(fls);
		}

		LiveRunner* runner = new LiveRunner();
		runner->initNm = "Init";
		runner->loopNm = "Loop";
		runner->path = IO::path + "/bin/liverunners/lj256/win32/lj256.dll";
		runner->name = "LJ256";
		LiveSyncer::runners.push_back(runner);

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
		glfwDestroyWindow(Display::window);
		//*/
	}
	catch (...) {
		std::cout << "Press enter to exit..." << std::endl;
		string s;
		std::getline(std::cin, s);
		return -1;
	}
}
