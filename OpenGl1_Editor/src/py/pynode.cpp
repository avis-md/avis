#include "pynode.h"

Font* PyNode::font = nullptr;
Texture* PyNode::tex_circle_open = nullptr, *PyNode::tex_circle_conn = nullptr;

PyNode::PyNode(PyScript* scr) : script(scr) {
	if (!scr) return;
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
	bool in = Rect(pos.x, pos.y, 220, 16).Inside(Input::mousePos);
	if (in) selected = true;
	return in;
}

void PyNode::DrawConn() {
	uint w = 220;
	auto cnt = (script->invarCnt + script->outvarCnt);
	float y = pos.y + 18;
	for (uint i = 0; i < script->invarCnt; i++, y += 17) {
		if (inputR[i]) Engine::DrawLine(Vec2(pos.x, y + 8), Vec2(inputR[i]->pos.x + w, inputR[i]->pos.y + 20 + 8 + (inputV[i].second + inputR[i]->script->invarCnt) * 17), white(), 2);
	}
}

void PyNode::Execute() {
	for (int i = 0; i < script->invarCnt; i++) {
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
			}
		}
	}
	script->Exec();
	for (int i = 0; i < script->outvarCnt; i++) {
		outputV[i].first.value = script->pRets[i];
		Py_INCREF(outputV[i].first.value);
	}
}

void PyNode::Draw() {
	uint w = 220;
	auto cnt = (script->invarCnt + script->outvarCnt);
	Engine::DrawQuad(pos.x, pos.y, w, 16, white(selected? 1.0f : 0.7f, 0.35f));
	Engine::DrawQuad(pos.x, pos.y + 16, w, 3 + 17 * cnt, white(0.7f, 0.25f));
	UI::Label(pos.x + 2, pos.y + 2, 12, script->name, font, white());
	float y = pos.y + 18;
	for (uint i = 0; i < script->invarCnt; i++, y += 17) {
		UI::Texture(pos.x - 5, y + 3, 10, 10, inputR[i] ? tex_circle_conn : tex_circle_open);
		UI::Label(pos.x + 10, y, 12, script->invars[i].name, font, white());
		if (!inputR[i]) {
			auto& vr = inputV[i].first;
			if (vr.type == PY_VARTYPE::INT || vr.type == PY_VARTYPE::FLOAT) {
				string s = std::to_string((vr.type == PY_VARTYPE::INT)? vr.ival : vr.fval);
				s = UI::EditText(pos.x + w * 0.33f, y, w * 0.67f - 6, 16, 12, white(1, 0.5f), s, font, true, nullptr, white());
				if ((vr.type == PY_VARTYPE::INT)) vr.ival = TryParse(s, 0);
				else vr.fval = TryParse(s, 0.0f);
			}
		}
	}
	y += 2;

	font->Align(ALIGN_TOPRIGHT);
	for (uint i = 0; i < script->outvarCnt; i++, y += 17) {
		UI::Texture(pos.x + w - 5, y + 3, 10, 10, outputR[i] ? tex_circle_conn : tex_circle_open);
		UI::Label(pos.x + w - 10, y, 12, script->outvars[i].name, font, white());
		UI::Label(pos.x + w * 0.66f, y, 12, script->outvars[i].typeName, font, white(0.3f), w * 0.67f - 6);
		/*
		if (!outputR[i]) {
			auto& vr = outputV[i].first;
			if (vr.type == PY_VARTYPE::INT || vr.type == PY_VARTYPE::FLOAT) {
				string s = std::to_string((vr.type == PY_VARTYPE::INT) ? vr.ival : vr.fval);
				s = UI::EditText(pos.x + w * 0.33f, y, w * 0.67f - 6, 16, 12, white(1, 0.5f), s, font, true, nullptr, white());
				if ((vr.type == PY_VARTYPE::INT)) vr.ival = TryParse(s, 0);
				else vr.fval = TryParse(s, 0.0f);
			}
			else {
				UI::Label(pos.x + w * 0.33f + 2, y, 12, script->outvars[i].typeName, font, white(0.3f), w * 0.67f - 6);
			}
		}*/
	}
	font->Align(ALIGN_TOPLEFT);
}