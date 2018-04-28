#include "PyWeb.h"

PyNode* PyWeb::selConnNode = nullptr;
uint PyWeb::selConnId = 0;
bool PyWeb::selConnIdIsOut = false, PyWeb::selPreClear = false;

std::vector<PyNode*> PyWeb::nodes;

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
		else {
			for (auto n : nodes) {
				if (n->selected) n->pos += Input::mouseDelta;
			}
		}
	}
}

void PyWeb::Draw() {
	PyWeb::selPreClear = true;
	for (auto n : nodes)
		n->DrawConn();
	for (auto n : nodes)
		n->Draw();
	if (Input::mouse0State == MOUSE_UP && selPreClear) selConnNode = nullptr;
}

void PyWeb::Execute() {
	for (auto n : nodes) n->Execute();
}