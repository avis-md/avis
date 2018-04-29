#include "PyWeb.h"
#include "pybrowse.h"
#include "ui/icons.h"

PyNode* PyWeb::selConnNode = nullptr;
uint PyWeb::selConnId = 0;
bool PyWeb::selConnIdIsOut = false, PyWeb::selPreClear = false;
PyScript* PyWeb::selScript = nullptr;

std::vector<PyNode*> PyWeb::nodes;

bool PyWeb::expanded = true, PyWeb::executing = false;
std::thread* PyWeb::execThread = nullptr;

void PyWeb::Insert(PyScript* scr, Vec2 pos) {
	Insert(new PyNode(scr), pos);
}

void PyWeb::Insert(PyNode* node, Vec2 pos) {
	nodes.push_back(node);
	nodes.back()->pos = pos;
}

void PyWeb::Update() {
	if (Input::mouse0) {
		if (Input::mouse0State == MOUSE_DOWN) {
			for (auto n : nodes) n->selected = false;
			for (auto n : nodes) {
				if (n->Select()) break;
			}
		}
		//else {
		//	for (auto n : nodes) {
		//		if (n->selected) n->pos += Input::mouseDelta;
		//	}
		//}
	}
	if (!executing && execThread) {
		if (execThread->joinable()) execThread->join();
	}
}

void PyWeb::Draw() {
	PyNode::width = 220;
	Engine::BeginStencil(PyBrowse::expanded ? 151 : 3, 0, Display::width, Display::height);
	byte ms = Input::mouse0State;
	if (executing) {
		Input::mouse0 = false;
		Input::mouse0State = 0;
	}
	PyWeb::selPreClear = true;
	Vec2 poss(PyBrowse::expanded ? 160 : 12, 100);
	float maxoff = 0, offy = -5;
	for (auto n : nodes) {
		if (!n->canTile) {
			poss.x += maxoff + 20;
			maxoff = 0;
		}
		else {
			poss.y += offy + 5;
		}
		n->pos = poss;
		auto o = n->DrawConn();
		maxoff = max(maxoff, o.x);
		offy = o.y;
	}
	for (auto n : nodes) {
		n->Draw();
	}
	if (Input::mouse0State == MOUSE_UP && selPreClear) selConnNode = nullptr;

	Input::mouse0State = ms;
	Input::mouse0 = (Input::mouse0State == 1) || (Input::mouse0State == 2);
	Engine::EndStencil();

	PyBrowse::Draw();

	if (Input::KeyDown(Key_Escape)) selScript = nullptr;
	if (selScript) UI::Texture(Input::mousePos.x - 16, Input::mousePos.y - 16, 32, 32, Icons::python, white(0.3f));
}

void PyWeb::DrawSide() {
	if (expanded) {
		float w = 180;
		PyNode::width = w - 2;
		Engine::DrawQuad(Display::width - w, 0, w, Display::height, white(0.8f, 0.2f));
		UI::Label(Display::width - w + 5, 3, 12, "Analysis", PyNode::font, white());

		if (Engine::Button(Display::width - 61, 1, 60, 16, white(1, 0.4f), "Run", 12, PyNode::font, white(), true) == MOUSE_RELEASE)
			Execute();

		//Engine::BeginStencil(Display::width - w, 0, 150, Display::height);
		Vec2 poss(Display::width - w + 1, 22);
		for (auto n : nodes) {
			n->pos = poss;
			poss.y += n->DrawSide();
		}
		//Engine::EndStencil();
		Engine::DrawQuad(Display::width - w - 16, Display::height - 16, 16, 16, white(1, 0.2f));
		if (Engine::Button(Display::width - w - 16, Display::height - 16, 16, 16, Icons::collapse, white(0.8f), white(), white(0.5f)) == MOUSE_RELEASE)
			expanded = false;
	}
	else {
		Engine::DrawQuad(Display::width - 2, 0, 2, Display::height, white(1, 0.2f));
		if (Engine::Button(Display::width - 112, Display::height - 16, 110, 16, white(1, 0.2f), white(1, 0.2f), white(1, 0.2f)) == MOUSE_RELEASE)
			expanded = true;
		UI::Texture(Display::width - 102, Display::height - 16, 16, 16, Icons::expand);
		UI::Label(Display::width - 86, Display::height - 15, 12, "Analysis", PyNode::font, white());
	}
}

void PyWeb::Execute() {
	if (!executing) {
		executing = true;
		if (execThread) delete(execThread);
		execThread = new std::thread(DoExecute);
	}
}

void PyWeb::DoExecute() {
	for (auto n : nodes) n->Execute();
	executing = false;
}