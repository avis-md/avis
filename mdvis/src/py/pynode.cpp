#include "PyWeb.h"
#include "ui/icons.h"

Font* PyNode::font = nullptr;
Texture* PyNode::tex_circle_open = nullptr, *PyNode::tex_circle_conn = nullptr;
float PyNode::width = 220;

PyNode::PyNode(PyScript* scr) : script(scr), canTile(false) {
	if (!scr) return;
	title = scr->name;
	inputR.resize(scr->invarCnt, nullptr);
	outputR.resize(scr->outvarCnt, nullptr);
	inputV.resize(scr->invarCnt);
	outputV.resize(scr->outvarCnt);
	for (uint i = 0; i < scr->invarCnt; i++) {
		inputV[i].first = PyVar(scr->invars[i]);
	}
	for (uint i = 0; i < scr->outvarCnt; i++) {
		outputV[i].first = PyVar(scr->outvars[i]);
	}
}

void PyNode::Init() {
	tex_circle_open = new Texture(IO::path + "/res/node_open.png");
	tex_circle_conn = new Texture(IO::path + "/res/node_conn.png");
}

bool PyNode::Select() {
	bool in = Rect(pos.x + 16, pos.y, width - 16, 16).Inside(Input::mousePos);
	if (in) selected = true;
	return in;
}

Vec2 PyNode::DrawConn() {
	auto cnt = (script->invarCnt + script->outvarCnt);
	float y = pos.y + 18;
	for (uint i = 0; i < script->invarCnt; i++, y += 17) {
		if (inputR[i]) Engine::DrawLine(Vec2(pos.x, expanded ? y + 8 : pos.y + 8), Vec2(inputR[i]->pos.x + inputR[i]->width, inputR[i]->expanded ? inputR[i]->pos.y + 20 + 8 + (inputV[i].second + inputR[i]->inputV.size()) * 17 : inputR[i]->pos.y + 8), white(), 2);
	}
	if (expanded) return Vec2(width, 19 + 17 * cnt);
	else return Vec2(width, 16);
}

void PyNode::Execute() {
	for (uint i = 0; i < script->invarCnt; i++) {
		if (inputR[i]) {
			script->invars[i].value = inputR[i]->outputV[inputV[i].second].first.value;
		}
		else {
			switch (script->invars[i].type) {
			case PY_VARTYPE::INT:
				script->SetVal(i, inputV[i].first.ival);
				break;
			case PY_VARTYPE::FLOAT:
				script->SetVal(i, inputV[i].first.fval);
				break;
			default:
				Debug::Error("PyNode", "Value not handled!");
				break;
			}
		}
	}
	script->Exec();
	for (uint i = 0; i < script->outvarCnt; i++) {
		outputV[i].first.value = script->pRets[i];
		Py_INCREF(outputV[i].first.value);
	}
}

void PyNode::ConnectTo(uint id, PyNode* tar, uint tarId) {
	if (outputV[id].first.typeName == tar->inputV[tarId].first.typeName) {
		if (tar->inputR[tarId]) tar->inputR[tarId]->outputR[tar->inputV[tarId].second] = nullptr;
		tar->inputR[tarId] = this;
		tar->inputV[tarId].second = id;
		outputR[id] = tar;
		outputV[id].second = tarId;
	}
}

void PyNode::Draw() {
	auto cnt = (script->invarCnt + script->outvarCnt);
	Engine::DrawQuad(pos.x, pos.y, width, 16, Vec4(titleCol, selected ? 1.0f : 0.7f));
	if (Engine::Button(pos.x, pos.y, 16, 16, expanded ? Icons::expand : Icons::collapse, white(0.8f), white(), white(0.5f)) == MOUSE_RELEASE) expanded = !expanded;
	UI::Label(pos.x + 18, pos.y + 1, 12, title, font, white());
	DrawToolbar();
	if (expanded) {
		Engine::DrawQuad(pos.x, pos.y + 16.0f, width, 3.0f + 17 * cnt, white(0.7f, 0.25f));
		float y = pos.y + 18;
		for (uint i = 0; i < script->invarCnt; i++, y += 17) {
			if (!PyWeb::selConnNode || (PyWeb::selConnIdIsOut && PyWeb::selConnNode != this)) {
				if (Engine::Button(pos.x - 5, y + 3, 10, 10, inputR[i] ? tex_circle_conn : tex_circle_open, white(), white(), white()) == MOUSE_RELEASE) {
					if (!PyWeb::selConnNode) {
						PyWeb::selConnNode = this;
						PyWeb::selConnId = i;
						PyWeb::selConnIdIsOut = false;
						PyWeb::selPreClear = false;
					}
					else {
						PyWeb::selConnNode->ConnectTo(PyWeb::selConnId, this, i);
						PyWeb::selConnNode = nullptr;
					}
				}
			}
			else if (PyWeb::selConnNode == this && PyWeb::selConnId == i && !PyWeb::selConnIdIsOut) {
				Engine::DrawLine(Vec2(pos.x, y + 8), Input::mousePos, white(), 1);
				UI::Texture(pos.x - 5, y + 3, 10, 10, inputR[i] ? tex_circle_conn : tex_circle_open);
			}
			else {
				UI::Texture(pos.x - 5, y + 3, 10, 10, inputR[i] ? tex_circle_conn : tex_circle_open, red(0.3f));
			}
			UI::Label(pos.x + 10, y, 12, script->invars[i].name, font, white());
			if (!inputR[i]) {
				auto& vr = inputV[i].first;
				if (vr.type == PY_VARTYPE::INT || vr.type == PY_VARTYPE::FLOAT) {
					string s = std::to_string((vr.type == PY_VARTYPE::INT) ? vr.ival : vr.fval);
					s = UI::EditText(pos.x + width * 0.33f, y, width * 0.67f - 6, 16, 12, white(1, 0.5f), s, font, true, nullptr, white());
					if (vr.type == PY_VARTYPE::INT) vr.ival = TryParse(s, 0);
					else vr.fval = TryParse(s, 0.0f);
				}
			}
		}
		y += 2;

		for (uint i = 0; i < script->outvarCnt; i++, y += 17) {
			if (!PyWeb::selConnNode || ((!PyWeb::selConnIdIsOut) && (PyWeb::selConnNode != this))) {
				if (Engine::Button(pos.x + width - 5, y + 3, 10, 10, outputR[i] ? tex_circle_conn : tex_circle_open, white(), white(), white()) == MOUSE_RELEASE) {
					if (!PyWeb::selConnNode) {
						PyWeb::selConnNode = this;
						PyWeb::selConnId = i;
						PyWeb::selConnIdIsOut = true;
						PyWeb::selPreClear = false;
					}
					else {
						ConnectTo(i, PyWeb::selConnNode, PyWeb::selConnId);
						PyWeb::selConnNode = nullptr;
					}
				}
			}
			else if (PyWeb::selConnNode == this && PyWeb::selConnId == i && PyWeb::selConnIdIsOut) {
				Engine::DrawLine(Vec2(pos.x + width, y + 8), Input::mousePos, white(), 1);
				UI::Texture(pos.x + width - 5, y + 3, 10, 10, outputR[i] ? tex_circle_conn : tex_circle_open);
			}
			else {
				UI::Texture(pos.x + width - 5, y + 3, 10, 10, outputR[i] ? tex_circle_conn : tex_circle_open, red(0.3f));
			}


			font->Align(ALIGN_TOPRIGHT);
			UI::Label(pos.x + width - 10, y, 12, script->outvars[i].name, font, white());
			font->Align(ALIGN_TOPLEFT);
			UI::Label(pos.x + 2, y, 12, script->outvars[i].typeName, font, white(0.3f), width * 0.67f - 6);
		}
		if (PyWeb::executing) Engine::DrawQuad(pos.x, pos.y + 16, width, 3.0f + 17 * cnt, white(0.5f, 0.25f));
	}
}

float PyNode::DrawSide() {
	auto cnt = (script->invarCnt);
	Engine::DrawQuad(pos.x, pos.y, width, 16, white(selected ? 1.0f : 0.7f, 0.35f));
	if (Engine::Button(pos.x, pos.y, 16, 16, expanded ? Icons::expand : Icons::collapse, white(0.8f), white(), white(0.5f)) == MOUSE_RELEASE) expanded = !expanded;
	UI::Label(pos.x + 20, pos.y + 1, 12, script->name, font, white());
	if (expanded) {
		Engine::DrawQuad(pos.x, pos.y + 16, width, 2.0f + 17 * cnt, white(0.7f, 0.25f));
		float y = pos.y + 18;
		for (uint i = 0; i < script->invarCnt; i++, y += 17) {
			UI::Label(pos.x + 2, y, 12, script->invars[i].name, font, white());
			if (!inputR[i]) {
				auto& vr = inputV[i].first;
				if (vr.type == PY_VARTYPE::INT || vr.type == PY_VARTYPE::FLOAT) {
					string s = std::to_string((vr.type == PY_VARTYPE::INT) ? vr.ival : vr.fval);
					s = UI::EditText(pos.x + width * 0.33f, y, width * 0.67f - 6, 16, 12, white(1, 0.5f), s, font, true, nullptr, white());
					if (vr.type == PY_VARTYPE::INT) vr.ival = TryParse(s, 0);
					else vr.fval = TryParse(s, 0.0f);
				}
			}
			else {
				UI::Label(pos.x + width * 0.33f, y, 12, "<connected>", font, yellow());
			}
		}
		if (PyWeb::executing) Engine::DrawQuad(pos.x, pos.y + 16, width, 3.0f + 17 * cnt, white(0.5f, 0.25f));
		return 19.0f + 17 * cnt;
	}
	else return 17.0f;
}

void PyNode::DrawToolbar() {
	if (Engine::Button(pos.x + width - 50, pos.y, 16, 16, Icons::left, white(0.8f), white(), white(0.5f)) == MOUSE_RELEASE) {
		op = PYNODE_OP::LEFT;
	}
	if (Engine::Button(pos.x + width - 33, pos.y, 16, 16, Icons::right, white(0.8f), white(), white(0.5f)) == MOUSE_RELEASE) {
		op = PYNODE_OP::RIGHT;
	}
	if (Engine::Button(pos.x + width - 16, pos.y, 16, 16, Icons::cross, white(0.8f), white(), white(0.5f)) == MOUSE_RELEASE) {
		op = PYNODE_OP::REMOVE;
	}
}