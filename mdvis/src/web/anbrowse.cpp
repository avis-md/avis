#include "anweb.h"
#include "ui/localizer.h"
#include "ui/popups.h"
#ifndef IS_ANSERVER
#include "ui/icons.h"
#include "ui/help.h"
#include "ui/ui_ext.h"
#include "utils/runcmd.h"
#endif

bool AnBrowse::busy = false;
std::string AnBrowse::busyMsg;
AnBrowse::Folder AnBrowse::folder = AnBrowse::Folder("");
bool AnBrowse::expanded = true;
bool AnBrowse::mscFdExpanded[] = {};
float AnBrowse::expandPos = 0;

AnBrowse::Folder* AnBrowse::doAddFd = nullptr;
std::string AnBrowse::tmplC, AnBrowse::tmplP, AnBrowse::tmplF;

void AnBrowse::DoScan(Folder* fo, const std::string& path, const std::string& incPath) {
	Debug::Message("AnBrowse", " Scanning folder: /" + incPath);
	fo->fullName = path;
	auto ff = IO::GetFiles(path, EXT_ANSV);
	for (auto f : ff) {
		auto nm = f.substr(f.find_last_of('/') + 1);
		fo->saves.push_back(nm.substr(0, nm.size() - EXT_ANSV_SZ));
	}
#define READ(ext, sz, cond, hd, pre)\
	ff = IO::GetFiles(path, ext);\
	for (auto f : ff) { \
		if (cond) {\
			pre\
			Debug::Message("AnBrowse", "  file: " + f);\
			f = f.substr(0, f.size() - sz);\
			auto iter = hd##Script::allScrs.find(f);\
			if (iter != hd##Script::allScrs.end())\
				Debug::Warning("AnBrowse", "Script name \"" + f + "\" already exists!");\
			else {\
				auto scr = new hd##Script();\
				fo->scripts.push_back(scr);\
				scr->name = f;\
				scr->path = incPath + scr->name;\
				scr->ok = false;\
				hd##Script::allScrs.emplace(f, scr);\
			}\
		}\
	}

	READ(EXT_PS, EXT_PS_SZ, f.substr(0, 2) != "__", Py, if (!PyReader::initd) {
		Debug::Message("System", "Initializing PyReader");
		PyReader::Init();
	});
	READ(EXT_CS, EXT_CS_SZ, 1, C,);
	READ(EXT_FS, EXT_FS_SZ, 1, F,);

	std::vector<std::string> fd;
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

void AnBrowse::Init() {
	tmplC = IO::GetText(IO::path + "res/templates/node_cpp");
	tmplP = IO::GetText(IO::path + "res/templates/node_py");
	tmplF = IO::GetText(IO::path + "res/templates/node_f90");
}

void AnBrowse::Scan() {
	Debug::Message("AnBrowse", "Scanning nodes folder...");
	folder = Folder(_("Custom"));
	DoScan(&folder, AnWeb::nodesPath, "");
	ErrorView::Refresh();
}

void AnBrowse::DoRefresh(Folder* fd) {
	for (auto s : fd->scripts) {
		s->busy = true;
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
		s->busy = false;
	}
	for (auto& f : fd->subfolders) {
		DoRefresh(&f);
	}
	if (fd == &folder) busy = false;
}

void AnBrowse::Refresh() {
	if (busy) return; //careful: this will not handle well if script is changed while compiling
	Debug::Message("AnBrowse", "Refreshing");
	busy = true;
	//DoRefresh(&folder);
	std::thread(DoRefresh, &folder).detach();
}

void AnBrowse::DoDraw(Folder* f, float& off, uint layer) {
#ifndef IS_ANSERVER
	UI::Quad(2.f + 5 * layer, off, 150, 16, white(1, 0.3f));
	if (Engine::Button(2.f + 5 * layer, off, 16, 16, f->expanded ? Icons::expand : Icons::collapse) == MOUSE_RELEASE)
		f->expanded = !f->expanded;
	if (!!Engine::Button(2.f + 5 * layer, off, 150.f, 16.f) || (doAddFd == f)) {
		if (Engine::Button(expandPos - 35, off + 1, 14, 14, Icons::expand) == MOUSE_RELEASE) {
			doAddFd = f;
			static std::vector<Popups::MenuItem> vm;
			vm.resize(4);
			vm[0].Set(0, "Folder", []() {
				IO::MakeDirectory(AnBrowse::doAddFd->fullName + "/newFolder");
				
			});
			vm[1].Set(0, "C++ Script", []() {
				std::string path = AnBrowse::doAddFd->fullName + "/";
				std::string nm = "newModule";
				int i = 2;
				while (IO::HasFile(path + nm + EXT_CS) ||
					(CScript::allScrs.find(nm) != CScript::allScrs.end())) {
					nm = "newModule" + std::to_string(i++);
				}
				std::ofstream strm(path + nm + EXT_CS);
				strm << AnBrowse::tmplC;
				strm.close();
				auto scr = new CScript();
				AnBrowse::doAddFd->scripts.push_back(scr);
				scr->name = nm;
				scr->path = AnBrowse::doAddFd->fullName.substr(IO::path.size() + 6) + nm;
				scr->ok = CReader::Read(scr);
				CScript::allScrs.emplace(nm, scr);
			});
			vm[2].Set(0, "Python Script", []() {
				std::string path = AnBrowse::doAddFd->fullName + "/";
				std::string nm = "newModule";
				int i = 2;
				while (IO::HasFile(path + nm + EXT_PS) ||
					(PyScript::allScrs.find(nm) != PyScript::allScrs.end())) {
					nm = "newModule" + std::to_string(i++);
				}
				std::ofstream strm(path + nm + EXT_PS);
				strm << AnBrowse::tmplP;
			});
			vm[3].Set(0, "Fortran Script", []() {
				std::string path = AnBrowse::doAddFd->fullName + "/";
				std::string nm = "newModule";
				int i = 2;
				while (IO::HasFile(path + nm + EXT_FS) ||
					(FScript::allScrs.find(nm) != FScript::allScrs.end())) {
					nm = "newModule" + std::to_string(i++);
				}
				std::ofstream strm(path + nm + EXT_FS);
				std::string scr = AnBrowse::tmplF;
				size_t pos;
				while ((pos = string_find(scr, "%NAME%")) != -1) {
					scr = scr.substr(0, pos) + to_uppercase(nm) + scr.substr(pos + 6);
				} 
				strm << scr;
			});

			Popups::pos = Vec2(expandPos - 35, off + 17);
			Popups::data = &vm;
			Popups::type = POPUP_TYPE::MENU;
		}
		if (Engine::Button(expandPos - 18, off, 16, 16, Icons::browse) == MOUSE_RELEASE) {
			IO::OpenFd(f->fullName);
		}
	}
	UI::Label(22.f + 5 * layer, off, 12.f, f->name, white());
	off += 17;
	if (f->expanded) {
		layer++;
		for (auto& fd : f->subfolders)
			DoDraw(&fd, off, layer);
		for (auto& fs : f->saves) {
			if (Engine::Button(2.f + 5 * layer, off, 150.f, 16.f, white(1, 0.35f)) == MOUSE_RELEASE) {
				AnWeb::Load(f->fullName + fs + EXT_ANSV);
			}
			UI::Texture(2.f + 5 * layer, off, 16.f, 16.f, Icons::icon_anl);
			UI::Label(22.f + 5 * layer, off, 12.f, fs, white());
			off += 17;
		}
		for (auto& fs : f->scripts) {
			if (Engine::Button(2.f + 5 * layer, off, 150.f, 16.f, white(1, 0.35f)) == MOUSE_RELEASE) {
				if (Input::dbclick) {
					AnWeb::selScript = nullptr;
					pAnNode pn;
					switch (fs->type) {
					case AN_SCRTYPE::PYTHON:
						pn = std::make_shared<PyNode>(dynamic_cast<PyScript*>(fs));
						break;
					case AN_SCRTYPE::C:
						pn = std::make_shared<CNode>(dynamic_cast<CScript*>(fs));
						break;
					case AN_SCRTYPE::FORTRAN:
						pn = std::make_shared<FNode>(dynamic_cast<FScript*>(fs));
						break;
					}
					pn->canTile = true;
					AnWeb::nodes.push_back(pn);
				}
				else {
					AnWeb::selScript = fs;
				}
			}
			Texture* icon = &Icons::lang_ft;
			if (fs->type == AN_SCRTYPE::C)
				icon = &Icons::lang_c;
			else if (fs->type == AN_SCRTYPE::PYTHON)
				icon = &Icons::lang_py;
			UI::Texture(2.f + 5 * layer, off, 16.f, 16.f, *icon);
			UI::Label(22.f + 5 * layer, off, 12.f, fs->name, white());
			if (!fs->ok) {
				if (fs->busy) {
					UI::Rotate(Time::time * 180, Vec2(137, off + 8));
					UI::Texture(129, off, 16, 16, Icons::refresh);
					UI::ResetMatrix();
				}
				else {
					UI::font->Align(ALIGN_TOPRIGHT);
					UI::Label(145, off, 12, std::to_string(fs->errorCount), red());
					UI::font->Align(ALIGN_TOPLEFT);
				}
			}
			off += 17;
		}
	}
#endif
}

void AnBrowse::Draw() {
#ifndef IS_ANSERVER
	if (Popups::type == POPUP_TYPE::NONE) doAddFd = nullptr;
	UI2::BackQuad(0, 0, expandPos, Display::height - 18.f);
	if (expanded) {
		UI::Label(5, 3, 12.f, "Scripts", white());
		HelpMenu::Link(expandPos - 16, 3, "anl/index.html");

		float f = UI::BeginScroll(0, 19, expandPos, Display::height - 38);
		for (int n = 0; n < ANNODE_GROUP_COUNT; ++n) {
			UI::Quad(2, f, 150.f, 16.f, white(1, 0.3f));
			if (Engine::Button(2, f, 16.f, 16.f, mscFdExpanded[n] ? Icons::expand : Icons::collapse) == MOUSE_RELEASE)
				mscFdExpanded[n] = !mscFdExpanded[n];
			UI::Label(22, f, 12.f, AnNode_Internal::groupNms[n], white());
			f += 17;
			if (mscFdExpanded[n]) {
				const auto& lst = AnNode_Internal::scrs[n];
				const size_t sz = lst.size();
				for (size_t a = 0; a < sz; a++) {
					if (Engine::Button(7, f, 150.f, 16.f, white(1, 0.35f)) == MOUSE_RELEASE) {
						AnWeb::selScript = (AnScript*)1;
						AnWeb::selSpNode = (n << 8) | a;
					}
					UI::Texture(7, f, 16.f, 16.f, Icons::lightning);
					UI::Label(27, f, 12.f, lst[a].name, white());
					f += 17;
				}
			}
		}

		DoDraw(&folder, f, 0);
		UI::EndScroll(f);

		UI2::BackQuad(expandPos, Display::height - 34.f, 16.f, 16.f);
		if ((!UI::editingText && Input::KeyUp(Key_S)) || Engine::Button(expandPos, Display::height - 34.f, 16.f, 16.f, Icons::collapse) == MOUSE_RELEASE)
			expanded = false;
		expandPos = std::min(expandPos + 1500 * Time::delta, 150.f);
	}
	else {
		if ((!UI::editingText && Input::KeyUp(Key_S)) || Engine::Button(expandPos, Display::height - 34.f, 110.f, 16.f, white(1, 0.2f), white(1, 0.2f), white(1, 0.2f)) == MOUSE_RELEASE)
			expanded = true;
		UI::Texture(expandPos, Display::height - 34.f, 16.f, 16.f, Icons::expand);
		UI::Label(expandPos + 18, Display::height - 33.f, 12.f, "Scripts (S)", white());
		expandPos = std::max(expandPos - 1500 * Time::delta, 0.f);
	}
#endif
}