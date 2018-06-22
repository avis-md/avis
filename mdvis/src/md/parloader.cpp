#include "parloader.h"
#include "Gromacs.h"
#include "Protein.h"
#include "vis/pargraphics.h"

std::vector<std::pair<std::vector<string>, string>> ParLoader::importers;

void ParLoader::Init() {
	ChokoLait::dropFuncs.push_back(OnDropFile);
}

bool ParLoader::Open(const char* path) {
	Gromacs::Read(string(path), false);

	Protein::Refresh();
	ParGraphics::UpdateDrawLists();
	return true;
}

bool ParLoader::OpenAnim(uint num, const char** paths) {

	return false;
}

void ParLoader::OnDropFile(int i, const char** c) {
	/*
	printf("File Dropped: \n");
	for (i--; i >= 0; i--) {
		printf("> %s\n", c[i]);
	}
	*/
	if (!Particles::particleSz) {
		for (i--; i >= 0; i--) {
			if (Open(c[i])) break;
		}
	}
}