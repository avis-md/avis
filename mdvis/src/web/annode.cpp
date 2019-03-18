#include "anweb.h"
#ifndef IS_ANSERVER
#include "md/particles.h"
#include "ui/icons.h"
#include "ui/ui_ext.h"
#include "res/resdata.h"
#endif
#include "vis/preferences.h"

Vec4 AnCol::conn_scalar = {};
Vec4 AnCol::conn_vector = {};

#define _script script->parent

void* AnNode::nodecon::getval() {
	return first->GetOutVal(second);
}

AnScript::Var& AnNode::nodecon::getvar() {
	return first->script->parent->outputs[second];
}

int AnNode::nodecon::getdim(int i) {
	return first->GetOutDim(second, i);
}

float AnNode::width = 220;

void AnNode::Init() {
	AnNode_Internal::Init();

	Preferences::Link("AVCS", &AnCol::conn_scalar);
	Preferences::Link("AVCV", &AnCol::conn_vector);
}

short& AnNode::getval_s(const uint i) {
	return inputR[i].first ? *(short*)inputR[i].getval() : script->defVals[i].s;
}
int& AnNode::getval_i(const uint i) {
	return inputR[i].first ? *(int*)inputR[i].getval() : script->defVals[i].i;
}
double& AnNode::getval_d(const uint i) {
	return inputR[i].first ? *(double*)inputR[i].getval() : script->defVals[i].d;
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
	return (showDesc ? (17 * _script->descLines) : 0) + (showSett ? setSz : 0) + hdrSz;
}

void AnNode::DrawBack() {
	if (AnWeb::highContrast) {
		UI2::BackQuad(pos.x, pos.y, width, height, white(1, 0.1f));
	}
	else {
		UI2::BackQuad(pos.x, pos.y, width, height);
		UI::Quad(pos.x, pos.y, width, height, white(0.7f, 0.1f));
	}
}

Vec2 AnNode::DrawConn() {
#ifndef IS_ANSERVER
	if (_script->ok) {
		const float connrad = 5;
		auto cnt = (_script->inputs.size() + _script->outputs.size());
		float y = pos.y + 18 + hdrSz;
		for (uint i = 0; i < _script->inputs.size(); i++, y += 17) {
			auto& ri = inputR[i];
			if (ri.first) {
				Vec2 p2 = Vec2(pos.x, expanded ? y + 8 : pos.y + 8);
				Vec2 p1 = Vec2(ri.first->pos.x + ri.first->width, ri.first->expanded ? ri.first->pos.y + 28 + ri.first->hdrSz + (ri.second + ri.first->inputR.size()) * 17 : ri.first->pos.y + 8);
				Vec2 hx = Vec2((p2.x > p1.x) ? (p2.x - p1.x) / 2 : (p1.x - p2.x) / 3, 0);
				auto col = AnWeb::highContrast? white(1, 0.5f) :
					Lerp(((ri.getvar().dim > 0) ? AnCol::conn_vector : AnCol::conn_scalar), black(), 0.3f);
				if (!AnWeb::selConnNode) {
					if (ri.hoverdel) {
						col = red();
					}
					else if (Rect(p1.x - connrad, p1.y - connrad, connrad * 2, connrad * 2).Inside(Input::mousePos)
						|| Rect(p2.x - connrad, p2.y - connrad, connrad * 2, connrad * 2).Inside(Input::mousePos)) {
						col = VisSystem::accentColor;
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

void AnNode::DrawMouseConn() {
	if (AnWeb::selConnNode) {
		const float connrad = 5;
		Vec2 p1 = Input::mousePos;
		Vec2 p2 = AnWeb::selConnPos;
		p2.x = AnWeb::selConnNode->pos.x + (AnWeb::selConnIdIsOut ? AnWeb::selConnNode->width : 0);
		Vec2 hx = Vec2((p2.x - p1.x) / 2, 0);
		if (AnWeb::selConnIdIsOut == (p1.x < p2.x)) hx *= -2.f/3;
		UI2::Bezier(p1, p1 + hx, p2 - hx, p2, VisSystem::accentColor, AnWeb::selConnCol, 5, 30);
		UI::Texture(p2.x - connrad, p2.y - connrad, connrad * 2, connrad * 2, Icons::circle, AnWeb::selConnCol);
	}
}

void AnNode::Draw() {
#ifndef IS_ANSERVER
	const float connrad = 5;

	auto cnt = _script->outputs.size();
	for (auto& i : inputR) {
		if (i.use) cnt++;
	}
	UI::Quad(pos.x, pos.y, width, 16, Vec4(titleCol, selected ? 1.f : 0.7f));
	if (Engine::Button(pos.x, pos.y, 16, 16, expanded ? Icons::expand : Icons::collapse, white(0.8f), white(), white(1, 0.5f)) == MOUSE_RELEASE) expanded = !expanded;
	UI::Label(pos.x + 18, pos.y + 1, 12, title, white());
	DrawToolbar();
	if (!_script->ok) {
		UI::Quad(pos.x, pos.y + 16, width, 19, white(0.7f, 0.25f));
		UI::Label(pos.x + 5, pos.y + 17, 12, std::to_string(_script->errorCount) + " errors", red());
	}
	else {
		if (expanded) {
			float y = pos.y + 16;
			if (AnWeb::highContrast) {
				UI::Quad(pos.x, y, width, height - 16, white(0.8f, 0.1f));
			}
			else {
				UI2::BlurQuad(pos.x, y, width, height - 16, white(0.5f));
				UI::Quad(pos.x, y, width, height - 16, white(0.6f, 0.2f));
			}
			y += 2;
			DrawHeader(y);
			hdrSz = y - pos.y - 16 - setSz;
			y += 1;
			for (uint i = 0; i < _script->inputs.size(); i++, y += 17) {
				bool hi = inputR[i].first;
				if (!inputR[i].use) {
					if (hi) Disconnect(i, false);
					continue;
				}
				auto cl0 = hi ? ((inputR[i].getvar().dim > 0) ? AnCol::conn_vector : AnCol::conn_scalar) : white();
				if (!AnWeb::selConnNode || (AnWeb::selConnIdIsOut && AnWeb::selConnNode != this)) {
					if (Engine::Button(pos.x - connrad, y + 8 - connrad, connrad * 2, connrad * 2, Icons::circle, 
							cl0, Vec4(1, 0.8f, 0.8f, 1), white(1, 0.5f)) == MOUSE_RELEASE) {
						if (!AnWeb::selConnNode) {
							AnWeb::selConnNode = this;
							AnWeb::selConnId = i;
							AnWeb::selConnIdIsOut = false;
							AnWeb::selPreClear = false;
							AnWeb::selConnPos = Vec2(pos.x, y + 8);
							AnWeb::selConnCol = cl0;
						}
						else {
							AnWeb::selConnNode->ConnectTo(AnWeb::selConnId, this, i);
							AnWeb::selConnNode = nullptr;
						}
					}
				}
				else if (AnWeb::selConnNode == this && AnWeb::selConnId == i && !AnWeb::selConnIdIsOut) {
					
				}
				else {
					UI::Texture(pos.x - connrad, y + 8 - connrad, connrad * 2, connrad * 2, Icons::circle, red(0.3f));
				}
				UI::Label(pos.x + 10, y, 12, _script->inputs[i].name, white());
				if (!HasConnI(i)) {
					auto& vr = _script->inputs[i].type;
					auto isi = (vr == AN_VARTYPE::INT);
					if (isi || (vr == AN_VARTYPE::DOUBLE)) {
						DrawDefVal(i, y);
					}
					else {
						UI::Label(pos.x + width*0.33f, y, 12, _script->inputs[i].typeName, white(0.3f));
					}
				}
				else {
					auto bt = hi ? Engine::Button(pos.x + width * 0.33f, y, width * 0.67f - 10, 16) : MOUSE_NONE;
					if (!bt) {
						UI::Label(pos.x + width * 0.33f, y, 12, "<connected>", yellow());
					}
					else {
						inputR[i].hoverdel = true;
						UI::Label(pos.x + width * 0.33f, y, 12, "disconnect", red(1, (bt == MOUSE_PRESS) ? 0.5f : 1));
						if (bt == MOUSE_RELEASE) {
							Disconnect(i, false);
						}
					}
				}
			}
			y += 2;

			for (uint i = 0; i < _script->outputs.size(); i++, y += 17) {
				bool ho = HasConnO(i);
				auto cl0 = ho ? ((_script->outputs[i].dim > 0) ? AnCol::conn_vector : AnCol::conn_scalar) : white();
				if (!AnWeb::selConnNode || ((!AnWeb::selConnIdIsOut) && (AnWeb::selConnNode != this))) {
					const auto cirbt = Engine::Button(pos.x + width - connrad, y + 8 - connrad, connrad * 2, connrad * 2, 
						Icons::circle, cl0, Vec4(1, 0.8f, 0.8f, 1), white(1, 0.5f));
					if ((cirbt & MOUSE_HOVER_FLAG) > 0) {
						std::string str = "";
						const auto& ov = script->parent->outputs[i];
						const auto& cv = conV[i];
						auto ptr = script->Resolve(cv.offset);
						if (!ptr) str = "failed to read variable";
						else {
							switch (ov.type) {
							case AN_VARTYPE::SHORT:
								str = std::to_string(*(short*)ptr);
								break;
							case AN_VARTYPE::INT:
								str = std::to_string(*(int*)ptr);
								break;
							case AN_VARTYPE::DOUBLE:
								str = std::to_string(*(double*)ptr);
								break;
							case AN_VARTYPE::LIST:
								if (!*(void**)ptr) str = "*0x0 (????)";
								else {
									str = "[";
									for (auto& d : cv.szOffsets) {
										str += std::to_string(*script->GetDimValue(d)) + "x";
									}
									str.back() = ']';
									str += " array (press F3 to view contents)";
									if (Input::KeyDown(KEY::F3)) {
										ArrayView::data = *(void**)ptr;
										ArrayView::type = ov.typeName[6];
										ArrayView::dims.clear();
										for (auto& d : cv.szOffsets) {
											ArrayView::dims.push_back(script->GetDimValue(d));
										}
										ArrayView::scrNm = title;
										ArrayView::varNm = ov.name;
										ArrayView::show = true;
									}
								}
								break;
							default:
								OHNO("AnNode::Draw", "Unexpected VARTYPE " + std::to_string((int)ov.type));
								return;
							}
						}
						UI2::Tooltip(cirbt, pos.x + width - connrad, y + 8 - connrad, str);
						if (cirbt == MOUSE_RELEASE) {
							if (!AnWeb::selConnNode) {
								AnWeb::selConnNode = this;
								AnWeb::selConnId = i;
								AnWeb::selConnIdIsOut = true;
								AnWeb::selPreClear = false;
								AnWeb::selConnPos = Vec2(pos.x + width, y + 8);
								AnWeb::selConnCol = cl0;
							}
							else {
								ConnectTo(i, AnWeb::selConnNode, AnWeb::selConnId);
								AnWeb::selConnNode = nullptr;
							}
						}
					}
				}
				else if (AnWeb::selConnNode == this && AnWeb::selConnId == i && AnWeb::selConnIdIsOut) {
				
				}
				else {
					UI::Texture(pos.x + width - connrad, y + 8 - connrad, connrad * 2, connrad * 2, Icons::circle, red(0.3f));
				}


				UI::font.Align(ALIGN_TOPRIGHT);
				auto bt = ho ? Engine::Button(pos.x + width*0.67f - 5, y, width * 0.33f, 16) : MOUSE_NONE;
				if (!bt) {
					UI::Label(pos.x + width - 10, y, 12, _script->outputs[i].name, white());
				}
				else {
					auto& ors = outputR[i];
					for (auto& o : ors) o.first->inputR[o.second].hoverdel = true;
					UI::Label(pos.x + width - 10, y, 12, "disconnect", red(1, (bt == MOUSE_PRESS) ? 0.5f : 1));
					if (bt == MOUSE_RELEASE) {
						Disconnect(i, true);
					}
				}
				UI::font.Align(ALIGN_TOPLEFT);
				UI::Label(pos.x + 2, y, 12, _script->outputs[i].typeName, white(0.3f), width * 0.67f - 6);
			}
			auto y1 = y;
			DrawFooter(y);
			ftrSz = y - y1;
			height = y - pos.y + 1;
			if (AnWeb::executing) {
				UI::Quad(pos.x, pos.y + 16, width, 3.f + 17 * cnt, white(0.5f, 0.25f));
				if (executing) UI::Quad(pos.x, pos.y + 16, 2 + (width - 2) * script->GetProgress(), 2, red());
			}
		}
		else height = 16;
	}
#endif
}

float AnNode::DrawSide() {
#ifndef IS_ANSERVER
	auto cnt = (_script->inputs.size());
	UI::Quad(pos.x, pos.y, width, 16, Vec4(titleCol, selected ? 1 : 0.8f));
	if (Engine::Button(pos.x, pos.y, 16, 16, expanded ? Icons::expand : Icons::collapse, white(0.8f), white(), white(0.5f)) == MOUSE_RELEASE) expanded = !expanded;
	UI::Label(pos.x + 20, pos.y + 1, 12, title, white());
	if (this->log.size() && Engine::Button(pos.x + width - 17, pos.y, 16, 16, Icons::log, white(0.8f), white(), white(0.5f)) == MOUSE_RELEASE) {
		logExpanded = !logExpanded;
	}
	if (executing) {
		UI::Quad(pos.x, pos.y + 16, width, 2, white(0.7f, 0.25f));
		UI::Quad(pos.x, pos.y + 16, 2 + (width - 2) * script->GetProgress(), 2, red());
	}
	float dy = (executing ? 18.f : 16.f), y = pos.y + dy;
	if (expanded) {
		UI::Quad(pos.x, y, width, height - dy, white(1, 0.2f));
		y += 2;
		DrawHeader(y);
		if (cnt > 0) {
			y += 2;
			for (uint i = 0; i < cnt; i++, y += 17) {
				UI::Label(pos.x + 2, y, 12, _script->inputs[i].name, white());
				auto& vr = _script->inputs[i].type;
				auto isi = (vr == AN_VARTYPE::INT);
				if (isi || (vr == AN_VARTYPE::DOUBLE)) {
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
						(rf ? rf->_script->outputs[rs].name : "no input");
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
								tmp = AnWeb::nodes[si - 1].get();
								ss.resize(1);
								ssi.clear();
								int k = 0;
								for (auto& v : tmp->_script->outputs) {
									if (CanConn(v.typeName, _script->inputs[i].typeName)) {
										ss.push_back(v.name);
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
	auto& vr = _script->inputs[i];
	auto& dr = script->defVals[i];
	auto isi = (vr.type == AN_VARTYPE::INT);
	auto old = dr.data;
	switch (vr.uiType) {
	case AnScript::Var::UI_TYPE::ENUM: {
		static Popups::DropdownItem di;
		UI2::Dropdown(pos.x + width*0.33f, y, width*0.67f - 6, di, [&]() {
			di.list = &vr.enums[0];
			di.target = (uint*)&dr.i;
		}, vr.enums[dr.i]);
		break;
	}
	case  AnScript::Var::UI_TYPE::RANGE: {
		float res = (isi ? (float)dr.i : (float)dr.d);
		res = UI2::Slider(pos.x + width*0.33f, y, width*0.67f - 6, vr.range.x, vr.range.y, res);
		if (isi) dr.i = (int)std::round(res);
		else dr.d = res;
		break;
	}
	default: {
		std::string s = isi ? std::to_string(dr.i) : std::to_string(dr.d);
		s = UI::EditText(pos.x + width * 0.33f, y, width * 0.67f - 6, 16, 12, white(1, 0.5f), s, true, white());
		if (isi) dr.i = TryParse(s, 0);
		else dr.d = TryParse(s, 0.0);
		break;
	}
	}
	if (dr.data != old) {
		OnValChange(i);
	}
}

void AnNode::DrawHeader(float& off) {
	if (showDesc) {
		UI::alpha = 0.7f;
		UI2::LabelMul(pos.x + 5, off, 12, _script->desc);
		UI::alpha = 1;
		off += std::roundf(12 * 1.2f * _script->descLines) + 2;
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
	if (!!_script->descLines) {
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
}

void AnNode::AddOutput(const CVar& cv) {
	outputR.push_back(std::vector<nodecon>());
	conV.push_back(cv);
	if (!(flags & AN_FLAG_NOSAVECONV)) conVAll.push_back(std::vector<VarVal>());
}

AnNode::AnNode(pAnScript_I scr) : flags(0), script(scr), canTile(false) {
	if (!scr) return;
	ResizeIO(scr->parent);
}

AnNode::AnNode(pAnScript_I scr, ANNODE_FLAGS flags) : flags(flags), script(scr), canTile(false) {}

void AnNode::ResizeIO(AnScript* scr) {
	inputR.resize(scr->inputs.size());
	auto osz = scr->outputs.size();
	outputR.resize(osz);
	conV.resize(osz);
	conVAll.resize(osz);
}

void AnNode::PreExecute() {
	execd = false;
	for (auto& i : inputR) {
		i.execd = !i.first;
	}
	log.clear();
}

bool AnNode::TryExecute() {
	std::lock_guard<std::mutex> lock(execMutex);
	if (execd) return false;
	if (!executing) {
		for (auto& i : inputR) {
			if (!i.execd) return false;
		}
	}
	{
		std::lock_guard<std::mutex> _lock(AnWeb::execNLock);
		AnWeb::execN++;
	}
	std::thread(_Execute, this).detach();
	return true;
}

void AnNode::_Execute(AnNode* n) {
	std::lock_guard<std::mutex> _lock(n->execMutex);
	auto lock = n->script->parent->allowParallel ? nullptr : &n->script->parent->parallelLock;
	if (lock) lock->lock();
	Debug::Message("AnNode", "Executing " + std::to_string(n->id) + n->title);
	n->executing = true;
	n->Execute();
	n->executing = false;
	n->execd = true;
	if (lock) lock->unlock();
	Debug::Message("AnNode", "Executed " + std::to_string(n->id) + n->title);
	for (size_t a = 0; a < n->outputR.size(); a++) {
		if (n->script->parent->outputs[a].type == AN_VARTYPE::LIST && n->outputR[a].size() > 0) {
			if (!*(void**)n->GetOutVal(a)) {
				Debug::Warning("AnNode", "Output " + std::to_string(a) + " of node " + n->title + " is (list)nullptr!");
				AnWeb::abortExec = true;
			}
		}
	}
	if (!AnWeb::abortExec)
		n->ExecuteNext();
	{
		std::lock_guard<std::mutex> _lock(AnWeb::execNLock);
		AnWeb::execN--;
	}
}

void AnNode::IAddConV(void* p) {
	auto _scr = (DmScript_I*)script.get();
	_scr->outputVs.push_back(VarVal());
	auto& vr = _scr->outputVs.back();
	vr.pval = p;
	conV.push_back(CVar());
	auto& cv = conV.back();
	cv.offset = (uintptr_t)(conV.size() - 1);
}

void AnNode::IAddConV(void* p, std::initializer_list<int*> d1, std::initializer_list<int> d2) {
	auto _scr = (DmScript_I*)script.get();
	_scr->outputVs.push_back(VarVal());
	auto& vr = _scr->outputVs.back();
	vr.val.p = p;
	vr.pval = &vr.val;
	conV.push_back(CVar());
	auto& cv = conV.back();
	cv.offset = (uintptr_t)(conV.size() - 1);
	auto pd1 = d1.begin();
	auto pd2 = d2.begin();
	while (pd1 != d1.end()) {
		if (!*pd1)
			cv.szOffsets.push_back(CVar::szItem(*pd2++));
		else
			cv.szOffsets.push_back(CVar::szItem(*pd1));
		pd1++;
	}
}

void AnNode::ExecuteNext() {
	for (auto& oo : outputR) {
		for (auto& o : oo) {
			o.first->inputR[o.second].execd = true;
			o.first->TryExecute();
		}
	}
}

void AnNode::ApplyFrameCount(int f) {
	for (auto& a : conVAll) {
		a.resize(f);
	}
}

void AnNode::WriteFrame(int f) {
	if (!!(flags & AN_FLAG_NOSAVECONV)) return;
	for (int a = 0; a < conV.size(); ++a) {
		auto& iv = script->parent->outputs[a];
		auto& c = conV[a];
		auto val = script->Resolve(c.offset);
		auto& ca = conVAll[a][f];
		switch (iv.type) {
		case AN_VARTYPE::SHORT:
			ca.val.i = *(short*)val;
			break;
		case AN_VARTYPE::INT:
			ca.val.i = *(int*)val;
			break;
		case AN_VARTYPE::DOUBLE:
			ca.val.d = *(double*)val;
			break;
		case AN_VARTYPE::LIST: {
			int n = 1;
			ca.dims.resize(iv.dim);
			for (size_t d = 0; d < iv.dim; ++d) {
				n *= ca.dims[d] = *script->GetDimValue(c.szOffsets[d]);
			}
			n *= iv.stride;
			ca.arr.resize(n);
			ca.val.p = ca.arr.data();
			memcpy(ca.val.p, *(char**)val, n);
			ca.pval = &ca.val.p;
			break;
		}
		}
	}
}

bool AnNode::ReadFrame(int f) {
	/*
	if (!(flags & AN_FLAG_NOSAVECONV) || !conVAll.size()) return true;
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
	*/
	return true;
}

void AnNode::RemoveFrames() {
	for (auto& a : conVAll) {
		a.clear();
	}
}



void AnNode::SaveOut(const std::string& path) {
	std::string nm = _script->name;
	std::ofstream strm(path + std::to_string(id) + nm, std::ios::binary);
	if (strm.is_open()) {
		auto cs = conV.size();
		_StreamWrite(&cs, &strm, 1);
		//for (auto& c : conV) {
			//c.Write(strm);
		//}
		strm.close();
	}
}

void AnNode::LoadOut(const std::string& path) {
	std::string nm = _script->name;
	std::ifstream strm(path + std::to_string(id) + nm, std::ios::binary);
	if (strm.is_open()) {
		byte sz = 0;
		_Strm2Val(strm, sz);
		if (sz != conV.size()) {
			Debug::Warning("Node", "output data corrupted!");
			return;
		}
		//for (auto& c : conV) {
			//c.Read(strm);
		//}
	}
}

void AnNode::SaveConn() {
	auto sz = inputR.size();
	_connInfo.resize(sz);
	for (size_t a = 0; a < sz; ++a) {
		ConnInfo& cn = _connInfo[a];
		cn.mynm = _script->inputs[a].name;
		cn.mytp = _script->inputs[a].typeName;
		auto ra = inputR[a].first;
		cn.cond = cn.tar = ra;
		if (cn.cond) {
			cn.tarnm = ra->_script->outputs[inputR[a].second].name;
			cn.tartp = ra->_script->outputs[inputR[a].second].typeName;
		}
	}
}

void AnNode::ClearConn() {
	for (size_t i = 0; i < inputR.size(); ++i) {
		Disconnect((uint)i, false);
	}
	inputR.clear();
	inputR.resize(_script->inputs.size());
	for (size_t i = 0; i < outputR.size(); ++i) {
		Disconnect((uint)i, true);
	}
	outputR.clear();
	outputR.resize(_script->outputs.size(), std::vector<nodecon>());
	conV.resize(outputR.size());
}

void AnNode::Reconn() {
	for (auto& cn : _connInfo) {
		for (size_t a = 0, sz = inputR.size(); a < sz; ++a) {
			if (_script->inputs[a].name == cn.mynm) {
				if (_script->inputs[a].typeName == cn.mytp) {
					if (cn.cond) {
						auto& ra = cn.tar;
						for (size_t b = 0, sz2 = ra->outputR.size(); b < sz2; ++b) {
							if (ra->_script->outputs[b].name == cn.tarnm) {
								if (ra->_script->outputs[b].typeName == cn.tartp) {
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
	auto& ov = _script->outputs[id];
	auto& it = tar->inputR[tarId];
	auto& iv = tar->_script->inputs[tarId];
	if (it.first == this && it.second == id) return;
	if (CanConn(ov.typeName, iv.typeName)) {
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
	msg.name = _script->name;
	msg.msg = ss;
	ErrorView::execMsgs.push_back(msg);
}

void AnNode::OnAnimFrame() {
	if (!!(flags & AN_FLAG_RUNONSEEK)) {
		//Debug::Message("AnNode", "autorun " + title);
		Execute();
	}
}

void AnNode::OnValChange(int i) {
	if (!!(flags & AN_FLAG_RUNONVALCHG)) {
		Execute();
	}
}

void* AnNode::GetOutVal(int i) {
	if (!AnWeb::executing && conVAll[i].size() > Particles::anim.currentFrame)
		return conVAll[i][Particles::anim.currentFrame].pval;
	else
		return script->Resolve(conV[i].offset);
}

int AnNode::GetOutDim(int i, int d) {
	if (!AnWeb::executing && conVAll[i].size() > Particles::anim.currentFrame)
		return conVAll[i][Particles::anim.currentFrame].dims[d];
	else
		return *script->GetDimValue(conV[i].szOffsets[d]);
}