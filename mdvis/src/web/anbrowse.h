#pragma once
#include "Engine.h"
#include "anweb.h"

class AnBrowse {
public:
	static bool busy;
	static std::string busyMsg;

	static struct Folder {
		Folder(std::string nm) : name(nm) {}

		std::string name, fullName;
		bool expanded = true;
		std::vector<AnScript*> scripts;
		std::vector<Folder> subfolders;
		std::vector<std::string> saves;
	} folder;

	static bool expanded;
	static float expandPos;
	static bool mscFdExpanded[10];

	static Folder* doAddFd;
	static std::string tmplC, tmplP, tmplF;

	static void Init();

	static void Scan(), DoScan(Folder* f, const std::string& path, const std::string& incPath);
	static void Refresh(), DoRefresh(Folder* f);

	static void Draw(), DoDraw(Folder* f, float& off, uint layer);
};