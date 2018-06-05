#include "anweb.h"
#include "anconv.h"
#include "ui/icons.h"

Font* AnNode::font = nullptr;
Texture* AnNode::tex_circle_open = nullptr, *AnNode::tex_circle_conn = nullptr;
float AnNode::width = 220;

void AnNode::Init() {
	tex_circle_open = new Texture(IO::path + "/res/node_open.png");
	tex_circle_conn = new Texture(IO::path + "/res/node_conn.png");

	Node_Volume::Init();
}

bool AnNode::Select() {
	bool in = Rect(pos.x + 16, pos.y, width - 16, 16).Inside(Input::mousePos);
	if (in) selected = true;
	return in;
}

Vec2 AnNode::DrawConn() {
	auto cnt = (script->invars.size() + script->outvars.size());
	float y = pos.y + 18;
	for (uint i = 0; i < script->invars.size(); i++, y += 17) {
		if (inputR[i].first) Engine::DrawLine(Vec2(pos.x, expanded ? y + 8 : pos.y + 8), Vec2(inputR[i].first->pos.x + inputR[i].first->width, inputR[i].first->expanded ? inputR[i].first->pos.y + 20 + 8 + (inputR[i].second + inputR[i].first->inputR.size()) * 17 : inputR[i].first->pos.y + 8), white(), 2);
	}
	if (expanded) return Vec2(width, 19 + 17 * cnt);
	else return Vec2(width, 16);
}

void AnNode::Draw() {
	auto cnt = (script->invars.size() + script->outvars.size());
	Engine::DrawQuad(pos.x, pos.y, width, 16, Vec4(titleCol, selected ? 1.0f : 0.7f));
	if (Engine::Button(pos.x, pos.y, 16, 16, expanded ? Icons::expand : Icons::collapse, white(0.8f), white(), white(0.5f)) == MOUSE_RELEASE) expanded = !expanded;
	UI::Label(pos.x + 18, pos.y + 1, 12, title, font, white());
	DrawToolbar();
	if (expanded) {
		Engine::DrawQuad(pos.x, pos.y + 16.0f, width, 3.0f + 17 * cnt, white(0.7f, 0.25f));
		float y = pos.y + 18;
		for (uint i = 0; i < script->invars.size(); i++, y += 17) {
			if (!AnWeb::selConnNode || (AnWeb::selConnIdIsOut && AnWeb::selConnNode != this)) {
				if (Engine::Button(pos.x - 5, y + 3, 10, 10, inputR[i].first ? tex_circle_conn : tex_circle_open, white(), white(), white()) == MOUSE_RELEASE) {
					if (!AnWeb::selConnNode) {
						AnWeb::selConnNode = this;
						AnWeb::selConnId = i;
						AnWeb::selConnIdIsOut = false;
						AnWeb::selPreClear = false;
					}
					else {
						AnWeb::selConnNode->ConnectTo(AnWeb::selConnId, this, i);
						AnWeb::selConnNode = nullptr;
					}
				}
			}
			else if (AnWeb::selConnNode == this && AnWeb::selConnId == i && !AnWeb::selConnIdIsOut) {
				Engine::DrawLine(Vec2(pos.x, y + 8), Input::mousePos, white(), 1);
				UI::Texture(pos.x - 5, y + 3, 10, 10, inputR[i].first ? tex_circle_conn : tex_circle_open);
			}
			else {
				UI::Texture(pos.x - 5, y + 3, 10, 10, inputR[i].first ? tex_circle_conn : tex_circle_open, red(0.3f));
			}
			UI::Label(pos.x + 10, y, 12, script->invars[i].first, font, white());
			if (!inputR[i].first) {
				auto& vr = script->invars[i].second;
				auto isi = (vr == "int");
				if (isi || (vr == "float")) {
					string s = std::to_string(isi ? inputVDef[i].i : inputVDef[i].f);
					s = UI::EditText(pos.x + width * 0.33f, y, width * 0.67f - 6, 16, 12, white(1, 0.5f), s, font, true, nullptr, white());
					if (isi) inputVDef[i].i = TryParse(s, 0);
					else inputVDef[i].f = TryParse(s, 0.0f);
				}
				else {
					UI::Label(pos.x + width * 0.33f, y, 12, script->invars[i].second, font, white(0.3f));
				}
			}
			else {
				UI::Label(pos.x + width * 0.33f, y, 12, "<connected>", font, yellow());
			}
		}
		y += 2;

		for (uint i = 0; i < script->outvars.size(); i++, y += 17) {
			if (!AnWeb::selConnNode || ((!AnWeb::selConnIdIsOut) && (AnWeb::selConnNode != this))) {
				if (Engine::Button(pos.x + width - 5, y + 3, 10, 10, outputR[i].first ? tex_circle_conn : tex_circle_open, white(), white(), white()) == MOUSE_RELEASE) {
					if (!AnWeb::selConnNode) {
						AnWeb::selConnNode = this;
						AnWeb::selConnId = i;
						AnWeb::selConnIdIsOut = true;
						AnWeb::selPreClear = false;
					}
					else {
						ConnectTo(i, AnWeb::selConnNode, AnWeb::selConnId);
						AnWeb::selConnNode = nullptr;
					}
				}
			}
			else if (AnWeb::selConnNode == this && AnWeb::selConnId == i && AnWeb::selConnIdIsOut) {
				Engine::DrawLine(Vec2(pos.x + width, y + 8), Input::mousePos, white(), 1);
				UI::Texture(pos.x + width - 5, y + 3, 10, 10, outputR[i].first ? tex_circle_conn : tex_circle_open);
			}
			else {
				UI::Texture(pos.x + width - 5, y + 3, 10, 10, outputR[i].first ? tex_circle_conn : tex_circle_open, red(0.3f));
			}


			font->Align(ALIGN_TOPRIGHT);
			UI::Label(pos.x + width - 10, y, 12, script->outvars[i].first, font, white());
			font->Align(ALIGN_TOPLEFT);
			UI::Label(pos.x + 2, y, 12, script->outvars[i].second, font, white(0.3f), width * 0.67f - 6);
		}
		if (AnWeb::executing) Engine::DrawQuad(pos.x, pos.y + 16, width, 3.0f + 17 * cnt, white(0.5f, 0.25f));
	}
}

float AnNode::DrawSide() {
	auto cnt = (script->invars.size());
	Engine::DrawQuad(pos.x, pos.y, width, 16, white(selected ? 1.0f : 0.7f, 0.35f));
	if (Engine::Button(pos.x, pos.y, 16, 16, expanded ? Icons::expand : Icons::collapse, white(0.8f), white(), white(0.5f)) == MOUSE_RELEASE) expanded = !expanded;
	UI::Label(pos.x + 20, pos.y + 1, 12, script->name, font, white());
	if (expanded) {
		Engine::DrawQuad(pos.x, pos.y + 16, width, 2.0f + 17 * cnt, white(0.7f, 0.25f));
		float y = pos.y + 18;
		for (uint i = 0; i < script->invars.size(); i++, y += 17) {
			UI::Label(pos.x + 2, y, 12, script->invars[i].first, font, white());
			if (!inputR[i].first) {
				auto& vr = script->invars[i].second;
				auto isi = (vr == "int");
				if (isi || (vr == "float")) {
					string s = std::to_string(isi ? inputVDef[i].i : inputVDef[i].f);
					s = UI::EditText(pos.x + width * 0.33f, y, width * 0.67f - 6, 16, 12, white(1, 0.5f), s, font, true, nullptr, white());
					if (isi) inputVDef[i].i = TryParse(s, 0);
					else inputVDef[i].f = TryParse(s, 0.0f);
				}
			}
			else {
				UI::Label(pos.x + width * 0.33f, y, 12, "<connected>", font, yellow());
			}
		}
		if (AnWeb::executing) Engine::DrawQuad(pos.x, pos.y + 16, width, 3.0f + 17 * cnt, white(0.5f, 0.25f));
		return 19.0f + 17 * cnt;
	}
	else return 17.0f;
}

void AnNode::DrawToolbar() {
	if (Engine::Button(pos.x + width - 50, pos.y, 16, 16, Icons::left, white(0.8f), white(), white(0.5f)) == MOUSE_RELEASE) {
		op = ANNODE_OP::LEFT;
	}
	if (Engine::Button(pos.x + width - 33, pos.y, 16, 16, Icons::right, white(0.8f), white(), white(0.5f)) == MOUSE_RELEASE) {
		op = ANNODE_OP::RIGHT;
	}
	if (Engine::Button(pos.x + width - 16, pos.y, 16, 16, Icons::cross, white(0.8f), white(), white(0.5f)) == MOUSE_RELEASE) {
		op = ANNODE_OP::REMOVE;
	}
}

void AnNode::ConnectTo(uint id, AnNode* tar, uint tarId) {
	if (script->outvars[id].second == tar->script->invars[tarId].second) {
		if (tar->inputR[tarId].first) tar->inputR[tarId].first->outputR[tar->inputR[tarId].second].first = nullptr;
		tar->inputR[tarId].first = this;
		tar->inputR[tarId].second = id;
		outputR[id].first = tar;
		outputR[id].second = tarId;
	}
}

AnNode::AnNode(AnScript* scr) : script(scr), canTile(false) {
	if (!scr) return;
	title = scr->name;
	inputR.resize(scr->invars.size(), std::pair<AnNode*, uint>());
	outputR.resize(scr->outvars.size(), std::pair<AnNode*, uint>());
	conV.resize(outputR.size());
}


PyNode::PyNode(PyScript* scr) : AnNode(scr) {
	if (!scr) return;
	title += " (python)";
	inputV.resize(scr->invars.size());
	outputV.resize(scr->outvars.size());
	inputVDef.resize(scr->invars.size());
	for (uint i = 0; i < scr->invars.size(); i++) {
		inputV[i] = scr->_invars[i];
		if (scr->_invars[i].type == AN_VARTYPE::FLOAT) inputVDef[i].f = 0;
		else inputVDef[i].i = 0;
	}
	for (uint i = 0; i < scr->outvars.size(); i++) {
		outputV[i] = scr->_outvars[i];
		switch (scr->_outvars[i].type) {
		case AN_VARTYPE::INT:
			conV[i].value = new int();
			break;
		case AN_VARTYPE::FLOAT:
			conV[i].value = new float();
			break;
		case AN_VARTYPE::LIST:
			conV[i].dimVals.resize(outputV[i].dim);
			for (int j = 0; j < outputV[i].dim; j++)
				conV[i].dimVals[j] = new int();
			break;
		}
	}
}

void PyNode::Execute() {
	auto scr = (PyScript*)script;
	for (uint i = 0; i < script->invars.size(); i++) {
		if (inputR[i].first) {
			scr->_invars[i].value = ((PyNode*)inputR[i].first)->outputV[inputR[i].second].value;
		}
		else {
			switch (scr->_invars[i].type) {
			case AN_VARTYPE::INT:
				script->Set(i, inputVDef[i].i);
				break;
			case AN_VARTYPE::FLOAT:
				script->Set(i, inputVDef[i].f);
				break;
			default:
				Debug::Error("PyNode", "Value not handled!");
				break;
			}
		}
	}
	script->Exec();
	for (uint i = 0; i < script->outvars.size(); i++) {
		outputV[i].value = scr->pRets[i];
		Py_INCREF(outputV[i].value);
		switch (outputV[i].type) {
		case AN_VARTYPE::FLOAT:
			*(float*)conV[i].value = PyFloat_AsDouble(outputV[i].value);
			break;
		case AN_VARTYPE::INT:
			*(float*)conV[i].value = PyFloat_AsDouble(outputV[i].value);
			break;
		case AN_VARTYPE::LIST:
			delete[]((float*)conV[i].value);
			conV[i].value = AnConv::FromPy(outputV[i].value, conV[i].dimVals.size(), &conV[i].dimVals[0]);
			break;
		}
	}
}


CNode::CNode(CScript* scr) : AnNode(scr) {
	if (!scr) return;
	title += " (c++)";
	inputV.resize(scr->invars.size());
	outputV.resize(scr->outvars.size());
	inputVDef.resize(scr->invars.size());
	for (uint i = 0; i < scr->invars.size(); i++) {
		inputV[i] = scr->_invars[i].value;
		if (scr->_invars[i].type == AN_VARTYPE::FLOAT) inputVDef[i].f = 0;
		else inputVDef[i].i = 0;
	}
	for (uint i = 0; i < scr->outvars.size(); i++) {
		outputV[i] = scr->_outvars[i].value;
		conV[i] = scr->_outvars[i];
	}
}

void CNode::Execute() {
	auto scr = (CScript*)script;
	for (uint i = 0; i < script->invars.size(); i++) {
		if (inputR[i].first) {
			auto& cv = inputR[i].first->conV[inputR[i].second];
			switch (scr->_invars[i].type) {
			case AN_VARTYPE::INT:
				script->Set(i, *((int*)cv.value));
				break;
			case AN_VARTYPE::FLOAT:
				script->Set(i, *((float*)cv.value));
				break;
			case AN_VARTYPE::LIST:
				auto& mv = scr->_invars[i];
				auto& vc = *((float**)mv.value) = (float*)cv.value;
				for (int j = 0; j < mv.dimVals.size(); j++) {
					*mv.dimVals[j] = *cv.dimVals[j];
				}
				break;
			//default:
			//	Debug::Error("CNode", "Value not handled!");
			//	break;
			}
		}
		else {
			switch (scr->_invars[i].type) {
			case AN_VARTYPE::INT:
				script->Set(i, inputVDef[i].i);
				break;
			case AN_VARTYPE::FLOAT:
				script->Set(i, inputVDef[i].f);
				break;
			default:
				Debug::Error("CNode", "Value not handled!");
				break;
			}
		}
	}

	script->Exec();


}