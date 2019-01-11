#include "anweb.h"
#include "anconv.h"
#ifndef IS_ANSERVER
#include "ui/icons.h"
#include "ui/ui_ext.h"
#include "res/resdata.h"
#endif

Texture AnNode::tex_circle_open, AnNode::tex_circle_conn;
float AnNode::width = 220;

short& AnNode::getval_s(const uint i) {
	return inputR[0].first ? *(short*)inputR[0].getval() : inputVDef[0].s;
}
int& AnNode::getval_i(const uint i) {
	return inputR[0].first ? *(int*)inputR[0].getval() : inputVDef[0].i;
}
double& AnNode::getval_d(const uint i) {
	return inputR[0].first ? *(double*)inputR[0].getval() : inputVDef[0].d;
}

void AnNode::Init() {
	AnNode_Internal::Init();
#ifndef IS_ANSERVER
	tex_circle_open = Texture(res::node_open_png, res::node_open_png_sz);
	tex_circle_conn = Texture(res::node_conn_png, res::node_conn_png_sz);
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

float AnNode::GetHeaderSz() {
	return (showDesc ? (17 * script->descLines) : 0) + (showSett ? setSz : 0) + hdrSz;
}

void AnNode::DrawBack() {
	UI2::BackQuad(pos.x, pos.y, width, height);
	if (expanded) UI::Quad(pos.x, pos.y, width, height, white(0.7f, 0.1f));
}

Vec2 AnNode::DrawConn() {
#ifndef IS_ANSERVER
	if (script->ok) {
		const float connrad = 6;
		auto cnt = (script->invars.size() + script->outvars.size());
		float y = pos.y + 18 + hdrSz;
		for (uint i = 0; i < script->invars.size(); i++, y += 17) {
			auto& ri = inputR[i];
			if (ri.first) {
				Vec2 p2 = Vec2(pos.x, expanded ? y + 8 : pos.y + 8);
				Vec2 p1 = Vec2(ri.first->pos.x + ri.first->width, ri.first->expanded ? ri.first->pos.y + 28 + ri.first->hdrSz + (ri.second + ri.first->inputR.size()) * 17 : ri.first->pos.y + 8);
				Vec2 hx = Vec2((p2.x > p1.x) ? (p2.x - p1.x) / 2 : (p1.x - p2.x) / 3, 0);
				auto col = white();
				if (!AnWeb::selConnNode) {
					if (ri.hoverdel) {
						col = red();
					}
					else if (Rect(p1.x - connrad, p1.y - connrad, connrad * 2, connrad * 2).Inside(Input::mousePos)
						|| Rect(p2.x - connrad, p2.y - connrad, connrad * 2, connrad * 2).Inside(Input::mousePos)) {
						col = Vec4(1, 0.7f, 0, 1);
					}
				}
				UI2::Bezier(p1, p1 + hx, p2 - hx, p2, col, 5, 30);
			}
			ri.hoverdel = false;
		}
		if (expanded) return Vec2(width, 19 + 17 * cnt + hdrSz + ftrSz + DrawLog(19 + 17 * cnt + hdrSz + ftrSz));
		else return Vec2(width, 16 + DrawLog(16));
	}
	else return Vec2(width, 35);
#else
	return Vec2();
#endif
}

void AnNode::Draw() {
#ifndef IS_ANSERVER
	const float connrad = 6;

	auto cnt = script->outvars.size();
	for (auto& i : inputR) {
		if (i.use) cnt++;
	}
	UI::Quad(pos.x, pos.y, width, 16, Vec4(titleCol, selected ? 1.f : 0.7f));
	if (Engine::Button(pos.x, pos.y, 16, 16, expanded ? Icons::expand : Icons::collapse, white(0.8f), white(), white(1, 0.5f)) == MOUSE_RELEASE) expanded = !expanded;
	UI::Label(pos.x + 18, pos.y + 1, 12, title, white());
	DrawToolbar();
	if (!script->ok) {
		UI::Quad(pos.x, pos.y + 16, width, 19, white(0.7f, 0.25f));
		UI::Label(pos.x + 5, pos.y + 17, 12, std::to_string(script->errorCount) + " errors", red());
	}
	else {
		if (expanded) {
			float y = pos.y + 16;
			UI2::BlurQuad(pos.x, y, width, height - 16, white(0.5f));
			UI::Quad(pos.x, y, width, height - 16, white(0.6f, 0.2f));
			y += 2;
			DrawHeader(y);
			hdrSz = y - pos.y - 16 - setSz;
			y += 1;
			for (uint i = 0; i < script->invars.size(); i++, y += 17) {
				bool hi = inputR[i].first;
				if (!inputR[i].use) {
					if (hi) Disconnect(i, false);
					continue;
				}
				if (!AnWeb::selConnNode || (AnWeb::selConnIdIsOut && AnWeb::selConnNode != this)) {
					if (Engine::Button(pos.x - connrad, y + 8-connrad, connrad*2, connrad*2, hi ? tex_circle_conn : tex_circle_open, white(), Vec4(1, 0.8f, 0.8f, 1), white(1, 0.5f)) == MOUSE_RELEASE) {
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
					Vec2 p2(pos.x, y + 8);
					Vec2 p1 = Input::mousePos;
					Vec2 hx = Vec2((p2.x > p1.x) ? (p2.x - p1.x)/2 : (p1.x - p2.x)/3, 0);
					UI2::Bezier(p1, p1 + hx, p2 - hx, p2, yellow(), 5, 30);
					UI::Texture(pos.x - connrad, y + 8-connrad, connrad*2, connrad*2, hi ? tex_circle_conn : tex_circle_open);
				}
				else {
					UI::Texture(pos.x - connrad, y + 8-connrad, connrad*2, connrad*2, hi ? tex_circle_conn : tex_circle_open, red(0.3f));
				}
				UI::Label(pos.x + 10, y, 12, script->invars[i].first, white());
				if (!HasConnI(i)) {
					auto& vr = script->invars[i].second;
					auto isi = (vr == "int");
					if (isi || (vr == "double")) {
						DrawDefVal(i, y);
					}
					else {
						UI::Label(pos.x + width*0.33f, y, 12, script->invars[i].second, white(0.3f));
					}
				}
				else {
					auto bt = hi? Engine::Button(pos.x + width * 0.33f, y, width * 0.67f - 10, 16) : MOUSE_NONE;
					if (!bt) {
						UI::Label(pos.x + width * 0.33f, y, 12, "<connected>", yellow());
					}
					else {
						inputR[i].hoverdel = true;
						UI::Label(pos.x + width * 0.33f, y, 12, "disconnect", red(1, (bt==MOUSE_PRESS)? 0.5f : 1));
						if (bt == MOUSE_RELEASE) {
							Disconnect(i, false);
						}
					}
				}
			}
			y += 2;

			for (uint i = 0; i < script->outvars.size(); i++, y += 17) {
				bool ho = HasConnO(i);
				if (!AnWeb::selConnNode || ((!AnWeb::selConnIdIsOut) && (AnWeb::selConnNode != this))) {
					const auto cirbt = Engine::Button(pos.x + width - connrad, y + 8-connrad, connrad*2, connrad*2, ho ? tex_circle_conn : tex_circle_open, white(), Vec4(1, 0.8f, 0.8f, 1), white(1, 0.5f));
					if ((cirbt & MOUSE_HOVER_FLAG) > 0) {
						std::string str = "";
						const auto& cv = conV[i];
						if (!cv.value) str = "0x0 (????)";
						else {
							switch (cv.type) {
							case AN_VARTYPE::SHORT:
								str = std::to_string(*(short*)cv.value);
								break;
							case AN_VARTYPE::INT:
								str = std::to_string(*(int*)cv.value);
								break;
							case AN_VARTYPE::DOUBLE:
								str = std::to_string(*(double*)cv.value);
								break;
							case AN_VARTYPE::LIST:
								str = "[";
								for (auto& d : cv.dimVals) {
									str += std::to_string(*d) + "x";
								}
								str.back() = ']';
								str += " array (press F3 to view contents)";
								if (Input::KeyDown(Key_F3)) {
									ArrayView::data = *(void**)cv.value;
									ArrayView::type = cv.typeName[6];
									ArrayView::dims = cv.dimVals;
									ArrayView::scrNm = title;
									ArrayView::varNm = script->outvars[i].first;
									ArrayView::show = true;
								}
								break;
							}
						}
						UI2::Tooltip(cirbt, pos.x + width - connrad, y + 8 - connrad, str);
						if (cirbt == MOUSE_RELEASE) {
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
				}
				else if (AnWeb::selConnNode == this && AnWeb::selConnId == i && AnWeb::selConnIdIsOut) {
					Vec2 p2 = Input::mousePos;
					Vec2 p1(pos.x + width, y + 8);
					Vec2 hx = Vec2((p2.x > p1.x) ? (p2.x - p1.x) / 2 : (p1.x - p2.x) / 3, 0);
					UI2::Bezier(p1, p1 + hx, p2 - hx, p2, yellow(), 5, 30);
					UI::Texture(pos.x + width - connrad, y + 8-connrad, connrad*2, connrad*2, ho ? tex_circle_conn : tex_circle_open);
				}
				else {
					UI::Texture(pos.x + width - connrad, y + 8-connrad, connrad*2, connrad*2, ho ? tex_circle_conn : tex_circle_open, red(0.3f));
				}


				UI::font->Align(ALIGN_TOPRIGHT);
				auto bt = ho? Engine::Button(pos.x + width*0.67f - 5, y, width * 0.33f, 16) : MOUSE_NONE;
				if (!bt) {
					UI::Label(pos.x + width - 10, y, 12, script->outvars[i].first, white());
				}
				else {
					auto& ors = outputR[i];
					for (auto& o : ors) o.first->inputR[o.second].hoverdel = true;
					UI::Label(pos.x + width - 10, y, 12, "disconnect", red(1, (bt==MOUSE_PRESS)? 0.5f : 1));
					if (bt == MOUSE_RELEASE) {
						Disconnect(i, true);
					}
				}
				UI::font->Align(ALIGN_TOPLEFT);
				UI::Label(pos.x + 2, y, 12, script->outvars[i].second, white(0.3f), width * 0.67f - 6);
			}
			auto y1 = y;
			DrawFooter(y);
			ftrSz = y - y1;
			height = y - pos.y + 1;
			if (AnWeb::executing) UI::Quad(pos.x, pos.y + 16, width, 3.f + 17 * cnt, white(0.5f, 0.25f));
		}
		else height = 16;
	}
#endif
}

float AnNode::DrawSide() {
#ifndef IS_ANSERVER
	auto cnt = (script->invars.size());
	UI::Quad(pos.x, pos.y, width, 16, Vec4(titleCol, selected ? 1 : 0.8f));
	if (Engine::Button(pos.x, pos.y, 16, 16, expanded ? Icons::expand : Icons::collapse, white(0.8f), white(), white(0.5f)) == MOUSE_RELEASE) expanded = !expanded;
	UI::Label(pos.x + 20, pos.y + 1, 12, title, white());
	if (this->log.size() && Engine::Button(pos.x + width - 17, pos.y, 16, 16, Icons::log, white(0.8f), white(), white(0.5f)) == MOUSE_RELEASE) {
		logExpanded = !logExpanded;
	}
	if (executing) {
		UI::Quad(pos.x, pos.y + 16, width, 2, white(0.7f, 0.25f));
		if (script->progress)
			UI::Quad(pos.x, pos.y + 16, 2 + (width - 2) * (float)(*script->progress), 2, red());
		else
			UI::Quad(pos.x, pos.y + 16, width * 0.1f, 2, red());
	}
	float dy = (executing ? 18 : 16), y = pos.y + dy;
	if (expanded) {
		UI::Quad(pos.x, y, width, height - dy, white(1, 0.2f));
		y += 2;
		DrawHeader(y);
		if (cnt > 0) {
			y += 2;
			for (uint i = 0; i < cnt; i++, y += 17) {
				UI::Label(pos.x + 2, y, 12, script->invars[i].first, white());
				auto& vr = script->invars[i].second;
				auto isi = (vr == "int");
				if (isi || (vr == "double")) {
					DrawDefVal(i, y);
				}
				else {
					auto& rf = inputR[i].first;
					auto& rs = inputR[i].second;
					static AnNode* tmp, *opt;
					static int opi;
					static Popups::DropdownItem di;
					static std::vector<std::string> ss = { "None" };
					static std::vector<int> ssi;
					static uint si;
					bool isme = (opt == this && opi == i);
					std::string tt = isme ?
						(tmp ? "Select variable" : "Select node") :
						(rf ? rf->script->outvars[rs].first : "no input");
					UI2::Dropdown(pos.x + width * 0.33f, y, width * 0.67f - 5, di, [&](){
						tmp = nullptr; opt = this; opi = i;
						for (uint a = 0; a < id; a++)
							ss.push_back(AnWeb::nodes[a]->title);
						ss.push_back("");
						di.list = &ss[0];
						di.target = &si;
						si = 0;
					}, tt, white(1, (rf && !isme) ? 1 : 0.5f));
					if (opt == this && opi == i) {
						if (Popups::type == POPUP_TYPE::NONE) {
							if (!di.seld) opt = nullptr;
							else if (!si) rf = opt = nullptr;
							else if (!tmp) {
								rf = nullptr;
								tmp = AnWeb::nodes[si - 1];
								ss.resize(1);
								ssi.clear();
								int k = 0;
								for (auto& v : tmp->script->outvars) {
									if (CanConn(v.second, script->invars[i].second)) {
										ss.push_back(v.first);
										ssi.push_back(k);
									}
									k++;
								}
								ss.push_back("");
								di.list = &ss[0];
								di.target = &si;
								si = 0;
								Popups::type = POPUP_TYPE::DROPDOWN;
							}
							else {
								tmp->ConnectTo(ssi[si - 1], this, i);
								opt = 0;
							}
						}
					}
				}
			}
		}
		if (AnWeb::executing) UI::Quad(pos.x, pos.y + 16, width, 3.f + 17 * cnt, white(0.5f, 0.25f));

		DrawFooter(y);

		y += 2;

		height = y - pos.y;

		return y - pos.y + 1 + DrawLog(y);
	}
	else return y - pos.y + 1 + DrawLog(y);
#else
	return 0;
#endif
}

void AnNode::DrawDefVal(int i, float y) {
	auto isi = (script->invars[i].second == "int");
	auto& opt = script->invaropts[i];
	auto old = inputVDef[i].data;
	switch (opt.type) {
	case VarOpt::ENUM: {
		static Popups::DropdownItem di;
		UI2::Dropdown(pos.x + width*0.33f, y, width*0.67f - 6, di, [&]() {
			di.list = &script->invaropts[i].enums[0];
			di.target = (uint*)&inputVDef[i].i;
		}, script->invaropts[i].enums[inputVDef[i].i]);
		break;
	}
	case VarOpt::RANGE: {
		float res = (float)(isi ? inputVDef[i].i : inputVDef[i].d);
		res = UI2::Slider(pos.x + width*0.33f, y, width*0.67f - 6, opt.range.x, opt.range.y, res);
		if (isi) inputVDef[i].i = (int)std::roundf(res);
		else inputVDef[i].d = res;
		break;
	}
	default: {
		std::string s = isi ? std::to_string(inputVDef[i].i) : std::to_string(inputVDef[i].d);
		s = UI::EditText(pos.x + width * 0.33f, y, width * 0.67f - 6, 16, 12, white(1, 0.5f), s, true, white());
		if (isi) inputVDef[i].i = TryParse(s, 0);
		else inputVDef[i].d = TryParse(s, 0.0);
		break;
	}
	}
	if (inputVDef[i].data != old) {
		OnValChange(i);
	}
}

void AnNode::DrawHeader(float& off) {
	if (showDesc) {
		UI::alpha = 0.7f;
		UI2::LabelMul(pos.x + 5, off, 12, script->desc);
		UI::alpha = 1;
		off += std::roundf(12 * 1.2f * script->descLines) + 2;
	}
	if (showSett) {
		float offo = off;
		DrawSettings(off);
		setSz = off - offo;
	}
	else setSz = 0;
}

float AnNode::DrawLog(float off) {
	auto sz = this->log.size();
	if (!sz) return 0;
	auto sz2 = std::min((int)sz, 10);
	if (logExpanded) {
		UI::Quad(pos.x, pos.y + off, width, 15.f * sz2 + 2, black(0.9f));
		Engine::PushStencil(pos.x + 1, pos.y + off, width - 2, 15.f * sz2);
		for (int i = 0; i < sz2; ++i) {
			auto& l = log[i + logOffset];
			Vec4 col = (!l.first) ? white() : ((l.first == 1) ? yellow() : red());
			UI::Label(pos.x + 4, pos.y + off + 1 + 15 * i, 12, l.second, col);
		}
		Engine::PopStencil();
		if (sz > 10) {
			float mw = 115;
			float of = logOffset * mw / sz;
			float w = 10 * mw / sz;
			UI::Quad(pos.x + width - 3, pos.y + off + 1 + of, 2, w, white(0.3f));
			if (Engine::Button(pos.x + width - 17, pos.y + off + 150 - 33, 16, 16, Icons::up, white(0.8f), white(), white(1, 0.5f)) == MOUSE_RELEASE) {
				logOffset = std::max(logOffset - 10, 0);
			}
			if (Engine::Button(pos.x + width - 17, pos.y + off + 150 - 16, 16, 16, Icons::down, white(0.8f), white(), white(1, 0.5f)) == MOUSE_RELEASE) {
				logOffset = std::min(logOffset + 10, (int)sz - 10);
			}
		}
		return 15 * sz2 + 2.f;
	}
	else {
		UI::Quad(pos.x, pos.y + off, width, 2, black(0.9f));
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

void AnNode::AddInput() {
	inputR.push_back(nodecon());
	inputVDef.push_back(AnVarBase());
}

void AnNode::AddOutput(const CVar& cv) {
	outputR.push_back(std::vector<nodecon>());
	conV.push_back(cv);
	if (saveConV) conVAll.push_back(std::vector<VarVal>());
}

AnNode::AnNode(AnScript* scr) : script(scr), canTile(false), 
		saveConV(true), runOnSeek(false), runOnValChg(false) {
	if (!scr) return;
	inputR.resize(scr->invars.size());
	auto osz = scr->outvars.size();
	outputR.resize(osz);
	conV.resize(osz);
	conVAll.resize(osz);
}

AnNode::AnNode(DmScript* scr, ANNODE_FLAGS flags) : script(scr), canTile(false), 
		saveConV(!(flags & AN_FLAG_NOSAVECONV)),
		runOnSeek(!!(flags & AN_FLAG_RUNONSEEK)),
		runOnValChg(!!(flags & AN_FLAG_RUNONVALCHG)) {}

void AnNode::ApplyFrameCount(int f) {
	for (auto& a : conVAll) {
		a.resize(f);
	}
}

void AnNode::WriteFrame(int f) {
	if (!saveConV) return;
	for (int a = 0; a < conV.size(); ++a) {
		auto& c = conV[a];
		auto& ca = conVAll[a][f];
		switch (c.type) {
		case AN_VARTYPE::SHORT:
			ca.val.i = *(short*)c.value;
			break;
		case AN_VARTYPE::INT:
			ca.val.i = *(int*)c.value;
			break;
		case AN_VARTYPE::DOUBLE:
			ca.val.d = *(double*)c.value;
			break;
		case AN_VARTYPE::LIST: {
			int n = 1;
			auto ds = c.dimVals.size();
			ca.dims.resize(ds);
			for (size_t d = 0; d < ds; ++d) {
				n *= ca.dims[d] = *c.dimVals[d];
			}
			n *= c.stride;
			ca.val.arr.data.resize(n);
			memcpy(&ca.val.arr.data[0], *(char**)c.value, n);
			ca.val.arr.p = &ca.val.arr.data[0];
			break;
		}
		}
	}
}

bool AnNode::ReadFrame(int f) {
	if (!saveConV || !conVAll.size()) return true;
	for (int a = 0; a < conV.size(); ++a) {
		auto& c = conV[a];
		if (conVAll[a].size() <= f) return false;
		auto& ca = conVAll[a][f];
		switch (c.type) {
		case AN_VARTYPE::INT:
			c.value = &ca.val.i;
			break;
		case AN_VARTYPE::DOUBLE:
			c.value = &ca.val.d;
			break;
		case AN_VARTYPE::LIST:
			for (int b = 0; b < c.dimVals.size(); ++b) {
				c.dimVals[b] = &ca.dims[b];
			}
			c.value = &ca.val.arr.p;
			break;
		}
	}
	return true;
}

void AnNode::RemoveFrames() {
	for (auto& a : conVAll) {
		a.clear();
	}
}



void AnNode::SaveOut(const std::string& path) {
	std::string nm = script->name;
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

void AnNode::LoadOut(const std::string& path) {
	std::string nm = script->name;
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

void AnNode::SaveConn() {
	auto sz = inputR.size();
	_connInfo.resize(sz);
	for (size_t a = 0; a < sz; ++a) {
		ConnInfo& cn = _connInfo[a];
		cn.mynm = script->invars[a].first;
		cn.mytp = script->invars[a].second;
		auto ra = inputR[a].first;
		cn.cond = cn.tar = ra;
		if (cn.cond) {
			cn.tarnm = ra->script->outvars[inputR[a].second].first;
			cn.tartp = ra->script->outvars[inputR[a].second].second;
		}
	}
}

void AnNode::ClearConn() {
	for (size_t i = 0; i < inputR.size(); ++i) {
		Disconnect((uint)i, false);
	}
	inputR.clear();
	inputR.resize(script->invars.size());
	for (size_t i = 0; i < outputR.size(); ++i) {
		Disconnect((uint)i, true);
	}
	outputR.clear();
	outputR.resize(script->outvars.size(), std::vector<nodecon>());
	conV.resize(outputR.size());
}

void AnNode::Reconn() {
	for (auto& cn : _connInfo) {
		for (size_t a = 0, sz = inputR.size(); a < sz; ++a) {
			if (script->invars[a].first == cn.mynm) {
				if (script->invars[a].second == cn.mytp) {
					if (cn.cond) {
						auto& ra = cn.tar;
						for (size_t b = 0, sz2 = ra->outputR.size(); b < sz2; ++b) {
							if (ra->script->outvars[b].first == cn.tarnm) {
								if (ra->script->outvars[b].second == cn.tartp) {
									ra->ConnectTo((uint)b, this, (uint)a);
									goto got;
								}
							}
						}
					}
					else goto got;
				}
			}
		}
		Debug::Message("AnNode", "skipping connection " + cn.tarnm + " -> " + cn.mynm);
		got:;
	}
}

bool AnNode::HasConnI(int i) {
	return inputR[i].first;
}
bool AnNode::HasConnO(int i) {
	return !!outputR[i].size();
}

bool AnNode::CanConn(std::string lhs, std::string rhs) {
	if (rhs[0] == '*') return true;
	auto s = lhs.size();
	if (s != rhs.size()) return false;
	for (size_t a = 0; a < s; ++a) {
		if (lhs[a] != rhs[a] && rhs[a] != '*') return false;
	}
	return true;
}

void AnNode::ConnectTo(uint id, AnNode* tar, uint tarId) {
	auto& ot = outputR[id];
	auto& ov = script->outvars[id];
	auto& it = tar->inputR[tarId];
	auto& iv = tar->script->invars[tarId];
	if (it.first == this && it.second == id) return;
	if (CanConn(ov.second, iv.second)) {
		if (it.first) tar->Disconnect(tarId, false);
		it.first = this;
		it.second = id;
		ot.push_back(nodecon(tar, tarId));
		OnConn(true, id);
		tar->OnConn(false, tarId);
	}
}

void AnNode::Disconnect(uint id, bool out) {
	if (out) {
		for (auto& a : outputR[id]) {
			if (a.first) a.first->inputR[a.second].first = nullptr;
		}
		outputR[id].clear();
	}
	else {
		if (!inputR[id].first) return;
		auto& ot = inputR[id].first->outputR[inputR[id].second];
		auto iter = std::find_if(ot.begin(), ot.end(), [this](nodecon n)->bool {
			return n.first == this;
		});
		assert(iter != ot.end());
		ot.erase(iter);
		inputR[id].first = nullptr;
	}
}

void AnNode::CatchExp(char* c) {
	auto ss = string_split(c, '\n');
	for (auto& s : ss) {
		log.push_back(std::pair<byte, std::string>(2, s));
	}

	ErrorView::Message msg{};
	msg.name = script->name;
	msg.msg = ss;
	ErrorView::execMsgs.push_back(msg);
}

void AnNode::OnAnimFrame() {
	if (runOnSeek) {
		//Debug::Message("AnNode", "autorun " + title);
		Execute();
	}
}

void AnNode::OnValChange(int i) {
	if (runOnValChg) {
		Execute();
	}
}