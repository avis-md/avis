#pragma once
#include "Engine.h"
#include "anweb.h"

class AnBrowse {
public:
	static struct Folder {
		Folder(string nm) : name(nm) {}

		string name, fullName;
		bool expanded = true;
		std::vector<AnScript*> scripts;
		std::vector<Folder> subfolders;
		std::vector<string> saves;
	} folder;

	static bool expanded;
	static float expandPos;

	static void Scan(), DoScan(Folder* f, const string& path, const string& incPath);
	static void Draw(), DoDraw(Folder* f, float& off, uint layer);
};