#include "parloader.h"
#include "Gromacs.h"
#include "Protein.h"
#include "vis/pargraphics.h"
#include "web/anweb.h"
#include "ui/icons.h"

bool ParLoader::loadAsTrj = false, ParLoader::additive = false;
int ParLoader::maxframes = 0;

std::vector<std::pair<std::vector<string>, string>> ParLoader::importers;

bool ParLoader::showDialog = false;
std::vector<string> ParLoader::droppedFiles;

bool ParLoader::_showImp = false;
float ParLoader::_impPos = 0, ParLoader::_impScr = 0;

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

void ParLoader::DrawOpenDialog() {
	if (!showDialog) return;
	UI::IncLayer();
	Engine::DrawQuad(0, 0, Display::width, Display::height, black(0.7f));

	float woff = Display::width*0.5f - 200 - _impPos / 2;
	float hoff = Display::height * 0.5f - 150;

	if (_showImp || _impPos > 0) {
		Engine::DrawQuad(woff + 400, hoff, _impPos, 300, white(0.8f, 0.1f));
		Engine::PushStencil(woff + 400, hoff, _impPos, 300);
		UI::Label(woff + 402, hoff, 12, "Choose Importer", AnNode::font, white());
		
		Engine::PopStencil();
	}
	_impPos = _showImp? min(_impPos + 800 * Time::delta, 100.0f) : max(_impPos - 800 * Time::delta, 0.0f);

	Engine::DrawQuad(woff, hoff, 400, 300, white(0.8f, 0.15f));
	Engine::DrawQuad(woff, hoff, 400, 16, white(0.9f, 0.1f));
	UI::Label(woff + 2, hoff, 12, loadAsTrj ? "Load Trajectory" : "Load Configuration", AnNode::font, white());
	UI::Label(woff + 2, hoff + 17, 12, "File(s)", AnNode::font, white());
	string nm = droppedFiles[0];
	if (Engine::Button(woff + 60, hoff + 17, 339, 16, white(1, 0.4f)) == MOUSE_RELEASE) {

	}
	UI::Label(woff + 62, hoff + 17, 12, nm, AnNode::font, white(0.7f), 326);
	UI::Texture(woff + 383, hoff + 17, 16, 16, Icons::browse);
	
	UI::Label(woff + 2, hoff + 34, 12, "Importer", AnNode::font, white(), 326);
	UI::Label(woff + 60, hoff + 34, 12, "Gromacs (.gro)", AnNode::font, white(0.5f), 326);
	if (Engine::Button(woff + 339, hoff + 34, 60, 16, white(1, 0.4f), _showImp ? "<<" : ">>", 12, AnNode::font, white(), true) == MOUSE_RELEASE) {
		_showImp = !_showImp;
	}
	
	//if (Particles::particleSz) {
		loadAsTrj = Engine::Toggle(woff + 1, hoff + 17 * 3, 16, Icons::checkbox, loadAsTrj, white(), ORIENT_HORIZONTAL);
		UI::Label(woff + 30, hoff + 17 * 3, 12, "As Trajectory", AnNode::font, white(), 326);
		additive = Engine::Toggle(woff + 201, hoff + 17 * 3, 16, Icons::checkbox, additive, white(), ORIENT_HORIZONTAL);
		UI::Label(woff + 230, hoff + 17 * 3, 12, "Additive", AnNode::font, white(), 326);
	//}
	UI::Label(woff + 2, hoff + 17 * 4, 12, "Options", AnNode::font, white(), 326);


	string line = "";
	if (loadAsTrj) line += "-trj ";
	if (additive) line += "-a";
	if (maxframes > 0) line += "-n" + std::to_string(maxframes) + " ";
	UI::Label(woff + 2, hoff + 300 - 17 * 2, 12, "Command line : " + line, AnNode::font, white(), 326);
	if (Engine::Button(woff + 300, hoff + 283, 48, 16, yellow(1, 0.4f), "Cancel", 12, AnNode::font, white(), true) == MOUSE_RELEASE) {
		showDialog = false;
	}
	if (Engine::Button(woff + 350, hoff + 283, 49, 16, white(0.4f), "Load", 12, AnNode::font, white(), true) == MOUSE_RELEASE) {
		//
		Gromacs::Read(droppedFiles[0], false);
		Protein::Refresh();
		ParGraphics::UpdateDrawLists();
		showDialog = false;
	}
}

bool ParLoader::OnDropFile(int i, const char** c) {
	if (AnWeb::drawFull) return false;
	droppedFiles.resize(i);
	for (i--; i >= 0; i--) {
		droppedFiles[i] = string(c[i]);
	}
	if (!Particles::particleSz) {
		
	}
	showDialog = true;
	return true;
}