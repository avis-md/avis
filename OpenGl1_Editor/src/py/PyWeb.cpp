#include "PyWeb.h"

std::vector<PyNode*> PyWeb::nodes;

void PyWeb::Insert(PyScript* scr, Vec2 pos) {
	nodes.push_back(new PyNode(scr));
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
	for (auto n : nodes)
		n->DrawConn();
	for (auto n : nodes)
		n->Draw();
}

void PyWeb::Execute() {
	for (auto n : nodes) 
}