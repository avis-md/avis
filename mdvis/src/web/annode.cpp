#include "anweb.h"
#include "anconv.h"
#ifndef IS_ANSERVER
#include "ui/icons.h"
#include "ui/ui_ext.h"
#include "res/resdata.h"
#endif

Texture* AnNode::tex_circle_open = nullptr, *AnNode::tex_circle_conn = nullptr;
float AnNode::width = 220;

void AnNode::Init() {
#ifndef IS_ANSERVER
	tex_circle_open = new Texture(res::node_open_png, res::node_open_png_sz);
	tex_circle_conn = new Texture(res::node_conn_png, res::node_conn_png_sz);

	Node_Volume::Init();
#endif
}

bool AnNode::Select() {
#ifndef IS_ANSERVER
	bool in = Rect(pos.x + 16, pos.y, width - 16, 16).Inside(Input::mousePos);
	if (in) selected = true;
	return in;
#else
	return false;
#endif
}

Vec2 AnNode::DrawConn() {
#ifndef IS_ANSERVER
	auto cnt = (script->invars.size() + script->outvars.size());
	float hy = GetHeaderSz();
	float y = pos.y + 18 + hy;
	for (uint i = 0; i < script->invars.size(); i++, y += 17) {
		auto& ri = inputR[i];
		if (ri.first) Engine::DrawLine(Vec2(pos.x, expanded ? y + 8 : pos.y + 8), Vec2(ri.first->pos.x + ri.first->width, ri.first->expanded ? ri.first->pos.y + 28 + ri.first->GetHeaderSz() + (ri.second + ri.first->inputR.size()) * 17 : ri.first->pos.y + 8), white(), 2);
	}
	if (expanded) return Vec2(width, 19 + 17 * cnt + DrawLog(19.0f + 17 * cnt) + hy + GetHeaderSz());
	else return Vec2(width, 16 + DrawLog(16));
#else
	return Vec2();
#endif
}

void AnNode::Draw() {
#ifndef IS_ANSERVER
	auto cnt = (script->invars.size() + script->outvars.size());
	Engine::DrawQuad(pos.x, pos.y, width, 16, Vec4(titleCol, selected ? 1.0f : 0.7f));
	if (Engine::Button(pos.x, pos.y, 16, 16, expanded ? Icons::expand : Icons::collapse, white(0.8f), white(), white(0.5f)) == MOUSE_RELEASE) expanded = !expanded;
	UI::Label(pos.x + 18, pos.y + 1, 12, title, white());
	DrawToolbar();
	if (expanded) {
		float y = pos.y + 18;
		DrawHeader(y);
		Engine::DrawQuad(pos.x, y - 2, width, 3.0f + 17 * cnt, white(0.7f, 0.25f));
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
			UI::Label(pos.x + 10, y, 12, script->invars[i].first, white());
			if (!inputR[i].first) {
				auto& vr = script->invars[i].second;
				auto isi = (vr == "int");
				if (isi || (vr == "float")) {
					string s = std::to_string(isi ? inputVDef[i].i : inputVDef[i].f);
					s = UI::EditText(pos.x + width * 0.33f, y, width * 0.67f - 6, 16, 12, white(1, 0.5f), s, true, white());
					if (isi) inputVDef[i].i = TryParse(s, 0);
					else inputVDef[i].f = TryParse(s, 0.0f);
				}
				else {
					UI::Label(pos.x + width * 0.33f, y, 12, script->invars[i].second, white(0.3f));
				}
			}
			else {
				UI::Label(pos.x + width * 0.33f, y, 12, "<connected>", yellow());
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


			UI::font->Align(ALIGN_TOPRIGHT);
			UI::Label(pos.x + width - 10, y, 12, script->outvars[i].first, white());
			UI::font->Align(ALIGN_TOPLEFT);
			UI::Label(pos.x + 2, y, 12, script->outvars[i].second, white(0.3f), width * 0.67f - 6);
		}
		if (AnWeb::executing) Engine::DrawQuad(pos.x, pos.y + 16, width, 3.0f + 17 * cnt, white(0.5f, 0.25f));
	}
#endif
}

float AnNode::GetHeaderSz() {
	return showDesc? (17 * script->descLines) : 0;
}

void AnNode::DrawHeader(float& off) {
	if (showDesc) {
		Engine::DrawQuad(pos.x, off, width, 17 * script->descLines, white(0.7f, 0.25f));
		UI::alpha = 0.7f;
		UI2::LabelMul(pos.x + 2, off + 1, 12, script->desc);
		UI::alpha = 1;
		off += 17 * script->descLines;
	}
}

float AnNode::DrawSide() {
#ifndef IS_ANSERVER
	auto cnt = (script->invars.size());
	Engine::DrawQuad(pos.x, pos.y, width, 16, white(selected ? 1.0f : 0.7f, 0.35f));
	if (Engine::Button(pos.x, pos.y, 16, 16, expanded ? Icons::expand : Icons::collapse, white(0.8f), white(), white(0.5f)) == MOUSE_RELEASE) expanded = !expanded;
	UI::Label(pos.x + 20, pos.y + 1, 12, title, white());
	if (this->log.size() && Engine::Button(pos.x + width - 17, pos.y, 16, 16, Icons::log, white(0.8f), white(), white(0.5f)) == MOUSE_RELEASE) {
		logExpanded = !logExpanded;
	}
	if (executing) {
		Engine::DrawQuad(pos.x, pos.y + 16, width, 2, white(0.7f, 0.25f));
		if (script->progress)
			Engine::DrawQuad(pos.x, pos.y + 16, 2 + (width - 2) * *((float*)script->progress), 2, red());
		else
			Engine::DrawQuad(pos.x, pos.y + 16, width * 0.1f, 2, red());
	}
	if (expanded) {
		Engine::DrawQuad(pos.x, pos.y + (executing? 18 : 16), width, 2.0f + 17 * cnt, white(0.7f, 0.25f));
		float y = pos.y + 18;
		for (uint i = 0; i < script->invars.size(); i++, y += 17) {
			UI::Label(pos.x + 2, y, 12, script->invars[i].first, white());
			if (!inputR[i].first) {
				auto& vr = script->invars[i].second;
				auto isi = (vr == "int");
				if (isi || (vr == "float")) {
					string s = std::to_string(isi ? inputVDef[i].i : inputVDef[i].f);
					s = UI::EditText(pos.x + width * 0.4f, y, width * 0.6f - 3, 16, 12, white(1, 0.5f), s, true, white());
					if (isi) inputVDef[i].i = TryParse(s, 0);
					else inputVDef[i].f = TryParse(s, 0.0f);
				}
				else {
					UI::Label(pos.x + width * 0.4f, y, 12, script->invars[i].second, white(0.3f));
				}
			}
			else {
				UI::Label(pos.x + width * 0.4f, y, 12, "<connected>", yellow());
			}
		}
		if (AnWeb::executing) Engine::DrawQuad(pos.x, pos.y + 16, width, 3.0f + 17 * cnt, white(0.5f, 0.25f));
		return 19.0f + 17 * cnt + DrawLog(18.0f + 17 * cnt);
	}
	else return 17.0f + DrawLog(16.0f);
#else
	return 0;
#endif
}

float AnNode::DrawLog(float off) {
	auto sz = this->log.size();
	if (!sz) return 0;
	auto sz2 = min<int>(sz, 10);
	if (logExpanded) {
		Engine::DrawQuad(pos.x, pos.y + off, width, 15.0f * sz2 + 2, black(0.9f));
		for (int i = 0; i < sz2; i++) {
			auto& l = log[i + logOffset];
			Vec4 col = (!l.first) ? white() : ((l.first == 1) ? yellow() : red());
			UI::Label(pos.x + 4, pos.y + off + 1 + 15 * i, 12, l.second, col);
		}
		if (sz > 10) {
			float mw = 115;
			float of = logOffset * mw / sz;
			float w = 10 * mw / sz;
			Engine::DrawQuad(pos.x + width - 3, pos.y + off + 1 + of, 2, w, white(0.3f));
			if (Engine::Button(pos.x + width - 17, pos.y + off + 150 - 33, 16, 16, Icons::up, white(0.8f), white(), white(1, 0.5f)) == MOUSE_RELEASE) {
				logOffset = max<int>(logOffset - 10, 0);
			}
			if (Engine::Button(pos.x + width - 17, pos.y + off + 150 - 16, 16, 16, Icons::down, white(0.8f), white(), white(1, 0.5f)) == MOUSE_RELEASE) {
				logOffset = min<int>(logOffset + 10, sz - 10);
			}
		}
		return 15 * sz2 + 2.0f;
	}
	else {
		Engine::DrawQuad(pos.x, pos.y + off, width, 2, black(0.9f));
		return 2;
	}
}

void AnNode::DrawToolbar() {
#ifndef IS_ANSERVER
	if (this->log.size() && Engine::Button(pos.x + width - 67, pos.y, 16, 16, Icons::log, white(0.8f), white(), white(0.5f)) == MOUSE_RELEASE) {
		logExpanded = !logExpanded;
	}
	if (!!script->descLines) {
		if (Engine::Button(pos.x + width - 33, pos.y, 16, 16, Icons::details, white(0.8f), white(), white(0.5f)) == MOUSE_RELEASE) {
			showDesc = !showDesc;
		}
	}
	if (Engine::Button(pos.x + width - 16, pos.y, 16, 16, Icons::cross, white(0.8f), white(), white(0.5f)) == MOUSE_RELEASE) {
		op = ANNODE_OP::REMOVE;
	}
#endif
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

#define sp << " "
#define nl << "\n"
void AnNode::Save(std::ofstream& strm) {
	string ext;
	strm << (int)script->type sp << (script->name + ext) sp;
	uint iv = 0;
	std::vector<int> is;
	std::vector<bool> ii;
	for (uint i = 0; i < script->invars.size(); i++) {
		if (script->invars[i].second == "int" || script->invars[i].second == "float") {
			iv++;
			is.push_back(i);
			ii.push_back(script->invars[i].second == "int");
		}
	}
	strm << iv sp << inputR.size() sp << (int)canTile nl;
	iv = 0;
	for (auto& i : is) {
		auto j = is[i];
		strm << script->invars[j].first sp << (ii[i]? inputVDef[j].i : inputVDef[j].f) nl;
	}

	int i = 0;
	for (auto& p : inputR) {
		if (p.first)
			strm << p.first->id sp << p.first->script->outvars[p.second].first sp << script->invars[i].first nl;
		else
			strm << "-1" nl;
		i++;
	}
}

void AnNode::Load(std::ifstream& strm) {
	int ic, cc, ct;
	strm >> ic >> cc >> ct;
	string vn, tp;
	for (auto i = 0; i < ic; i++) {
		strm >> vn >> tp;
		for (uint b = 0; b < script->invars.size(); b++) {
			string nn = script->invars[b].first;
			if (nn == vn) {
				auto t = ((CScript*)script)->_invars[b].type;
				if (t == AN_VARTYPE::FLOAT)
					inputVDef[b].f = TryParse(tp, 0.0f);
				else if (t == AN_VARTYPE::INT)
					inputVDef[b].i = TryParse(tp, 0);
			}
		}
	}
	for (auto i = 0; i < cc; i++) {
		strm >> ic;
		if (ic < 0) continue;
		strm >> vn >> tp;
		AnNode* n2 = AnWeb::nodes[ic];
		uint i1 = -1, i2 = -1;
		for (uint b = 0; b < n2->script->outvars.size(); b++) {
			if (n2->script->outvars[b].first == vn) {
				i2 = b;
				break;
			}
		}
		for (uint b = 0; b < script->invars.size(); b++) {
			if (script->invars[b].first == tp) {
				i1 = b;
				break;
			}
		}
		if (i1 == -1 || i2 == -1) continue;
		n2->ConnectTo(i2, this, i1);
	}
	canTile = !!ct;
}

void AnNode::SaveOut(const string& path) {
	string nm = script->name;
	//std::replace(nm.begin(), nm.end(), '/', '_');
	std::ofstream strm(path + std::to_string(id) + nm, std::ios::binary);
	if (strm.is_open()) {
		auto cs = conV.size();
		_StreamWrite(&cs, &strm, 1);
		for (auto& c : conV) {
			c.Write(strm);
		}
		strm.close();
	}
}

void AnNode::LoadOut(const string& path) {
	string nm = script->name;
	std::ifstream strm(path + std::to_string(id) + nm, std::ios::binary);
	if (strm.is_open()) {
		byte sz = 0;
		_Strm2Val(strm, sz);
		if (sz != conV.size()) {
			Debug::Warning("Node", "output data corrupted!");
			return;
		}
		for (auto& c : conV) {
			c.Read(strm);
		}
	}
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
		default:
			Debug::Warning("PyNode", "case not handled!");
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
		default:
			Debug::Warning("AnVar", "exec case not handled!");
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
			auto& mv = scr->_invars[i];
			switch (mv.type) {
			case AN_VARTYPE::INT:
				script->Set(i, *((int*)cv.value));
				break;
			case AN_VARTYPE::FLOAT:
				script->Set(i, *((float*)cv.value));
				break;
			case AN_VARTYPE::LIST:
				*((float**)mv.value) = *((float**)cv.value);
				
				for (uint j = 0; j < mv.dimVals.size(); j++) {
					*mv.dimVals[j] = *cv.dimVals[j];
				}
				break;
			default:
				Debug::Warning("CNode", "var not handled!");
				break;
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