#include "anweb.h"
#include "ui/localizer.h"
#ifndef IS_ANSERVER
#include "ui/icons.h"
#include "utils/runcmd.h"
#endif

AnBrowse::Folder AnBrowse::folder = AnBrowse::Folder("");
bool AnBrowse::expanded = true;
bool AnBrowse::mscFdExpanded[] = {};
float AnBrowse::expandPos = 0;

void AnBrowse::DoScan(Folder* fo, const string& path, const string& incPath) {
	Debug::Message("AnBrowse", " Scanning folder: /" + incPath);
	fo->fullName = path;
	auto ff = IO::GetFiles(path, EXT_ANSV);
	for (auto f : ff) {
		auto nm = f.substr(f.find_last_of('/') + 1);
		fo->saves.push_back(nm.substr(0, nm.size() - EXT_ANSV_SZ));
	}
#define READ(ext, sz, cond, hd)\
	ff = IO::GetFiles(path, ext);\
	for (auto f : ff) { \
		auto nm = f.substr(f.find_last_of('/') + 1);\
		if (cond) {\
			Debug::Message("AnBrowse", "  file: " + f);\
			auto scr = new hd##Script();\
			fo->scripts.push_back(scr);\
			scr->path = incPath + nm.substr(0, nm.size() - sz);\
			scr->ok = hd##Reader::Read(scr);\
		}\
	}

	READ(EXT_PS, EXT_PS_SZ, nm.substr(0, 2) == "__", Py);
	READ(EXT_CS, EXT_CS_SZ, 1, C);
	READ(EXT_FS, EXT_FS_SZ, 1, F);

	std::vector<string> fd;
	IO::GetFolders(path, &fd);

	for (auto f : fd) {
		if (f.substr(0, 2) == "__") continue;
		fo->subfolders.push_back(Folder(f));
		auto& bk = fo->subfolders.back();
		DoScan(&bk, path + f + "/", incPath + f + "/");
		if (!bk.scripts.size() && !bk.subfolders.size())
			fo->subfolders.pop_back();
	}
}

void AnBrowse::Scan() {
	Debug::Message("AnBrowse", "Scanning nodes folder...");
	folder = Folder(_("Custom"));
	DoScan(&folder, IO::path + "nodes", "");
	ErrorView::Refresh();
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
		case AN_SCRTYPE::FORTRAN:
			FReader::Refresh((FScript*)s);
			break;
		default:
			OHNO("AnBrowse::DoRefresh", "Invalid script type " + std::to_string((int)s->type));
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
	UI::Quad(2.0f + 5 * layer, off, 150.0f, 16.0f, white(1, 0.3f));
	if (Engine::Button(2.0f + 5 * layer, off, 16.0f, 16.0f, f->expanded ? Icons::expand : Icons::collapse) == MOUSE_RELEASE)
		f->expanded = !f->expanded;
	if (!!Engine::Button(2.0f + 5 * layer, off, 150.0f, 16.0f)) {
		if (Engine::Button(expandPos - 18, off, 16, 16, Icons::browse) == MOUSE_RELEASE) {
			IO::OpenFd(f->fullName);
		}
	}
	UI::Label(22.0f + 5 * layer, off, 12.0f, f->name, white());
	off += 17;
	if (f->expanded) {
		layer++;
		for (auto& fd : f->subfolders)
			DoDraw(&fd, off, layer);
		for (auto& fs : f->saves) {
			if (Engine::Button(2.0f + 5 * layer, off, 150.0f, 16.0f, white(1, 0.35f)) == MOUSE_RELEASE) {
				AnWeb::Load(f->fullName + fs + EXT_ANSV);
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
			else
				icon = Icons::lang_ft;
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
	UI::Quad(0.0f, 0.0f, expandPos, Display::height - 18.0f, white(0.9f, 0.15f));
	if (expanded) {
		float f = 20;
		Engine::BeginStencil(0.0f, 0.0f, expandPos, Display::height - 18.0f);
		UI::Label(5.0f, 3.0f, 12.0f, "Scripts", white());

#define BT(nm) (byte)(AN_NODE_ ## nm)
#define MSC1(n, nm) UI::Quad(2, f, 150.0f, 16.0f, white(1, 0.3f)); \
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
		UI::Quad(expandPos, Display::height - 34.0f, 16.0f, 16.0f, white(1, 0.2f));
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