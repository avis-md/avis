#include "pybrowse.h"
#include "ui/icons.h"
#include "pynode.h"
#include "PyWeb.h"

PyBrowse::Folder PyBrowse::folder = Folder("py");
bool PyBrowse::expanded = true;
float PyBrowse::expandPos = 150;

void PyBrowse::DoScan(Folder* fo, const string& path, const string& incPath) {
	auto ff = IO::GetFiles(path, ".py");

	for (auto f : ff) {
		auto nm = f.substr(f.find_last_of('\\') + 1);
		if (nm.substr(0, 2) == "__") continue;
		fo->scripts.push_back(nullptr);
		PyReader::Read(incPath + nm.substr(0, nm.size() - 3), &fo->scripts.back());
	}

	std::vector<string> fd;
	IO::GetFolders(path, &fd);

	for (auto f : fd) {
		if (f == "." || f == "..") continue;
		fo->subfolders.push_back(Folder(f));
		DoScan(&fo->subfolders.back(), path + "\\" + f, incPath + f + "/");
		if (!fo->subfolders.back().scripts.size() && !fo->subfolders.back().subfolders.size())
			fo->subfolders.pop_back();
	}
}

void PyBrowse::Scan() {
	DoScan(&folder, IO::path + "\\py", "");
}

void PyBrowse::DoDraw(Folder* f, float& off, uint layer) {
	Engine::DrawQuad(2 + 5 * layer, off, 150, 16, white(1, 0.3f));
	if (Engine::Button(2 + 5 * layer, off, 16, 16, f->expanded ? Icons::expand : Icons::collapse, white(0.8f), white(), white(0.5f)) == MOUSE_RELEASE)
		f->expanded = !f->expanded;
	UI::Label(22 + 5 * layer, off + 1, 12, f->name, PyNode::font, white());
	off += 17;
	if (f->expanded) {
		layer++;
		for (auto& fd : f->subfolders)
			DoDraw(&fd, off, layer);
		for (auto& fs : f->scripts) {
			if (Engine::Button(2 + 5 * layer, off, 150, 16, white(1, 0.35f)) == MOUSE_RELEASE) {
				PyWeb::selScript = fs;
			}
			UI::Texture(2 + 5 * layer, off, 16, 16, Icons::python);
			UI::Label(22 + 5 * layer, off + 1, 12, fs->name, PyNode::font, white());
			off += 17;
		}
	}
}

void PyBrowse::Draw() {
	if (expanded) {
		Engine::DrawQuad(0, 0, expandPos, Display::height, white(0.9f, 0.15f));
		float f = 20;
		Engine::BeginStencil(0, 0, expandPos, Display::height);
		UI::Label(5, 3, 12, "Python Files", PyNode::font, white());
		DoDraw(&folder, f, 0);
		Engine::EndStencil();
		Engine::DrawQuad(expandPos, Display::height - 16, 16, 16, white(1, 0.2f));
		if (Engine::Button(expandPos, Display::height - 16, 16, 16, Icons::collapse, white(0.8f), white(), white(0.5f)) == MOUSE_RELEASE)
			expanded = false;
		expandPos = min(expandPos + 1000 * Time::delta, 150.0f);
	}
	else {
		Engine::DrawQuad(0, 0, expandPos, Display::height, white(0.8f, 0.15f));
		if (Engine::Button(expandPos, Display::height - 16, 110, 16, white(1, 0.2f), white(1, 0.2f), white(1, 0.2f)) == MOUSE_RELEASE)
			expanded = true;
		UI::Texture(expandPos, Display::height - 16, 16, 16, Icons::expand);
		UI::Label(expandPos + 18, Display::height - 15, 12, "Python Files", PyNode::font, white());
		expandPos = max(expandPos - 1000 * Time::delta, 0.0f);
	}
}