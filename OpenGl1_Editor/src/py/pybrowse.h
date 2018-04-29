#pragma once
#include "Engine.h"
#include "pyreader.h"

class PyBrowse {
public:
	static struct Folder {
		Folder(string nm) : name(nm) {}

		string name;
		bool expanded = true;
		std::vector<PyScript*> scripts;
		std::vector<Folder> subfolders;
	} folder;
	
	static bool expanded;

	static void Scan(), DoScan(Folder* f, const string& path, const string& incPath);
	static void Draw(), DoDraw(Folder* f, float& off, uint layer);
};