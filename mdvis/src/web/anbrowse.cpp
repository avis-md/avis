#include "anweb.h"
#include "ui/localizer.h"
#ifndef IS_ANSERVER
#include "ui/icons.h"
#include "utils/runcmd.h"
#endif

#ifdef PLATFORM_WIN
   #define stat _stat
#endif

AnBrowse::Folder AnBrowse::folder = AnBrowse::Folder("");
bool AnBrowse::expanded = true;
bool AnBrowse::mscFdExpanded[] = {};
float AnBrowse::expandPos = 0;

void AnBrowse::DoScan(Folder* fo, const string& path, const string& incPath) {
	auto ff = IO::GetFiles(path, ".anl");
	fo->saves.reserve(ff.size());
	for (auto f : ff) {
		string nm = f.substr(f.find_last_of('/') + 1);
		fo->saves.push_back(incPath + nm.substr(0, nm.size() - 4));
	}
	ff = IO::GetFiles(path, ".py");
	for (auto f : ff) {
		auto nm = f.substr(f.find_last_of('/') + 1);
		if (nm.substr(0, 2) == "__") continue;
		fo->scripts.push_back(new PyScript());
		auto scr = fo->scripts.back();
		scr->path = incPath + nm.substr(0, nm.size() - 3);
		scr->ok = PyReader::Read(scr->path, (PyScript*)scr);
	}

	ff = IO::GetFiles(path, ".cpp");
	if (ff.size() && !IO::HasDirectory(path + "/__ccache__/"))
		IO::MakeDirectory(path + "/__ccache__/");

	for (auto f : ff) {
		auto nm = f.substr(f.find_last_of('/') + 1);
		fo->scripts.push_back(new CScript());
		auto scr = fo->scripts.back();
		scr->path = incPath + nm.substr(0, nm.size() - 4);
		scr->ok = CReader::Read(scr->path, (CScript*)scr);
	}

	std::vector<string> fd;
	IO::GetFolders(path, &fd);

	for (auto f : fd) {
		fo->subfolders.push_back(Folder(f));
		auto& bk = fo->subfolders.back();
		DoScan(&bk, path + "/" + f, incPath + f + "/");
		if (!bk.scripts.size() && !bk.subfolders.size())
			fo->subfolders.pop_back();
		bk.fullName = incPath + f;
	}
}

void AnBrowse::Scan() {
	folder = Folder(_("Custom"));
	DoScan(&folder, IO::path + "/nodes", "");
}

void AnBrowse::DoRefresh(Folder* fd) {
	for (auto s : fd->scripts) {
		switch (s->type) {
		case AN_SCRTYPE::C:
			CReader::Refresh((CScript*)s);
			break;
		case AN_SCRTYPE::PYTHON:
			PyReader::Refresh((PyScript*)s);
			break;
		default:

			break;
		}
	}
	for (auto& f : fd->subfolders) {
		DoRefresh(&f);
	}
}

void AnBrowse::Refresh() {
	DoRefresh(&folder);
}

void AnBrowse::DoDraw(Folder* f, float& off, uint layer) {
#ifndef IS_ANSERVER
	Engine::DrawQuad(2.0f + 5 * layer, off, 150.0f, 16.0f, white(1, 0.3f));
	if (Engine::Button(2.0f + 5 * layer, off, 16.0f, 16.0f, f->expanded ? Icons::expand : Icons::collapse) == MOUSE_RELEASE)
		f->expanded = !f->expanded;
	UI::Label(22.0f + 5 * layer, off, 12.0f, f->name, white());
	off += 17;
	if (f->expanded) {
		layer++;
		for (auto& fd : f->subfolders)
			DoDraw(&fd, off, layer);
		for (auto& fs : f->saves) {
			if (Engine::Button(2.0f + 5 * layer, off, 150.0f, 16.0f, white(1, 0.35f)) == MOUSE_RELEASE) {
				
			}
			UI::Texture(2.0f + 5 * layer, off, 16.0f, 16.0f, Icons::icon_anl);
			UI::Label(22.0f + 5 * layer, off, 12.0f, fs, white());
			off += 17;
		}
		for (auto& fs : f->scripts) {
			if (Engine::Button(2.0f + 5 * layer, off, 150.0f, 16.0f, white(1, 0.35f)) == MOUSE_RELEASE) {
				AnWeb::selScript = fs;
			}
			Texture* icon = 0;
			if (fs->type == AN_SCRTYPE::C)
				icon = Icons::lang_c;
			else if (fs->type == AN_SCRTYPE::PYTHON)
				icon = Icons::lang_py;
			UI::Texture(2.0f + 5 * layer, off, 16.0f, 16.0f, icon);
			UI::Label(22.0f + 5 * layer, off, 12.0f, fs->name, white());
			if (!fs->ok) {
				UI::font->Align(ALIGN_TOPRIGHT);
				UI::Label(145.0f - 5 * layer, off, 12, std::to_string(fs->errorCount), red());
				UI::font->Align(ALIGN_TOPLEFT);
			}
			off += 17;
		}
	}
#endif
}

void AnBrowse::Draw() {
#ifndef IS_ANSERVER
	Engine::DrawQuad(0.0f, 0.0f, expandPos, Display::height - 18.0f, white(0.9f, 0.15f));
	if (expanded) {
		float f = 20;
		Engine::BeginStencil(0.0f, 0.0f, expandPos, Display::height - 18.0f);
		UI::Label(5.0f, 3.0f, 12.0f, "Scripts", white());

#define BT(nm) (byte)(AN_NODE_ ## nm)
#define MSC1(n, nm) Engine::DrawQuad(2, f, 150.0f, 16.0f, white(1, 0.3f)); \
		if (Engine::Button(2, f, 16.0f, 16.0f, mscFdExpanded[n] ? Icons::expand : Icons::collapse) == MOUSE_RELEASE) \
			mscFdExpanded[n] = !mscFdExpanded[n]; \
		UI::Label(22, f, 12.0f, nm, white()); \
		f += 17; \
		if (mscFdExpanded[n])
#define MSC2() if (Engine::Button(7, f, 150.0f, 16.0f, white(1, 0.35f)) == MOUSE_RELEASE) { \
					AnWeb::selScript = (AnScript*)1; \
					AnWeb::selSpNode = a; \
				} \
				UI::Texture(7, f, 16.0f, 16.0f, Icons::lightning);

		MSC1(0, "Scene IO") {
			for (byte a = BT(SCN::NUM0) + 1; a < BT(SCN::NUM); a++) {
				MSC2()
				UI::Label(27, f, 12.0f, AN_NODE_SCNS[a - BT(SCN::NUM0) - 1], white());
				f += 17;
			}
		}

		MSC1(1, _("Inputs")) {
			for (byte a = BT(IN::NUM0) + 1; a < BT(IN::NUM); a++) {
				MSC2()
				UI::Label(27, f, 12.0f, AN_NODE_INS[a - BT(IN::NUM0) - 1], white());
				f += 17;
			}
		}

		MSC1(2, _("Modifiers")) {
			for (byte a = BT(MOD::NUM0) + 1; a < BT(MOD::NUM); a++) {
				MSC2()
				UI::Label(27, f, 12.0f, AN_NODE_MODS[a - BT(MOD::NUM0) - 1], white());
				f += 17;
			}
		}

		MSC1(3, _("Generators")) {
			for (byte a = BT(GEN::NUM0) + 1; a < BT(GEN::NUM); a++) {
				MSC2()
				UI::Label(27, f, 12.0f, AN_NODE_GENS[a - BT(GEN::NUM0) - 1], white());
				f += 17;
			}
		}

		MSC1(4, _("Miscellaneous")) {
			for (byte a = BT(MISC::NUM0) + 1; a < BT(MISC::NUM); a++) {
				MSC2()
				UI::Label(27, f, 12.0f, AN_NODE_MISCS[a - BT(MISC::NUM0) - 1], white());
				f += 17;
			}
		}

		DoDraw(&folder, f, 0);
		Engine::EndStencil();
		Engine::DrawQuad(expandPos, Display::height - 34.0f, 16.0f, 16.0f, white(1, 0.2f));
		if ((!UI::editingText && Input::KeyUp(Key_S)) || Engine::Button(expandPos, Display::height - 34.0f, 16.0f, 16.0f, Icons::collapse) == MOUSE_RELEASE)
			expanded = false;
		expandPos = min(expandPos + 1500 * Time::delta, 150.0f);
	}
	else {
		if ((!UI::editingText && Input::KeyUp(Key_S)) || Engine::Button(expandPos, Display::height - 34.0f, 110.0f, 16.0f, white(1, 0.2f), white(1, 0.2f), white(1, 0.2f)) == MOUSE_RELEASE)
			expanded = true;
		UI::Texture(expandPos, Display::height - 34.0f, 16.0f, 16.0f, Icons::expand);
		UI::Label(expandPos + 18, Display::height - 33.0f, 12.0f, "Scripts (S)", white());
		expandPos = max(expandPos - 1500 * Time::delta, 0.0f);
	}
#endif
}

void AnBrowse::PreC(bool f) {
	DoPreC(IO::path + "/nodes/", f);
}

void AnBrowse::PreFt(bool f) {
	
}

void AnBrowse::DoPreC(const string& path, bool _f) {
	auto ff = IO::GetFiles(path, ".cpp");
	if (ff.size() && !IO::HasDirectory(path + "__ccache__/"))
		IO::MakeDirectory(path + "__ccache__/");

	for (auto f : ff) {
		//auto nm = f.substr(f.find_last_of('/') + 1);
		auto nm = f.substr(0, f.find_last_of('.'));

		if (!_f) {
			string srf = path + f;
			string obf = path + "__ccache__/" + nm + ".so";
			struct stat tsf, tof;
			if (!stat(srf.c_str(), & tsf) && !stat(obf.c_str(), & tof)) {
				if (tsf.st_mtime < tof.st_mtime) {
					std::cout << "Skipping " << path + nm << std::endl;
					continue;
				}
			}
		}
		
		std::cout << "Compiling " << path + nm << std::endl;

		auto s = IO::GetText(path + f);
		s = "\
#define VARIN extern \"C\"\n\
#define VAROUT VARIN\n\
#define ENTRY VARIN\n\
#define VECSZ(...)\n\
#define VECVAR VARIN\n\
\n" + s;

		const string ss = path + nm + "_temp__.cpp";
		std::ofstream ostrm(ss);
		ostrm << s;
		ostrm.close();

		const string cmd = "g++ -std=c++11 -shared -O0 -fPIC -o \"" + path + "__ccache__/" + nm + ".so\" \"" + path + nm + "_temp__.cpp\"";
		std::cout << cmd << std::endl;
		RunCmd::Run(cmd);
		remove(&ss[0]);
	}

	std::vector<string> fd;
	IO::GetFolders(path, &fd);

	for (auto f : fd) {
		DoPreC(path + "/" + f, _f);
	}
}

void AnBrowse::DoPreFt() {
	
}