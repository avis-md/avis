#pragma once
#include "Engine.h"
#include "pyreader.h"

class PyBrowse {
public:
	static struct Folder {
		Folder(string nm) : name(nm) {}

		string name;
		std::vector<PyScript*> scripts;
		std::vector<Folder> subfolders;
	} folder;
	
	static void Scan(), DoScan(Folder* f, const string& path, const string& incPath);
};
