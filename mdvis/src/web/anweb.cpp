#include "anweb.h"
#ifndef IS_ANSERVER
#include "anops.h"
#include "ui/icons.h"
#include "md/Particles.h"
#include "vis/pargraphics.h"
#endif

AnNode* AnWeb::selConnNode = nullptr;
uint AnWeb::selConnId = 0;
bool AnWeb::selConnIdIsOut = false, AnWeb::selPreClear = false;
AnScript* AnWeb::selScript = nullptr;
byte AnWeb::selSpNode = 0;

std::vector<AnNode*> AnWeb::nodes;

bool AnWeb::drawFull = false, AnWeb::expanded = true, AnWeb::executing = false;
float AnWeb::maxScroll, AnWeb::scrollPos = 0, AnWeb::expandPos = 0;

std::thread* AnWeb::execThread = nullptr;

bool AnWeb::hasPy = true, AnWeb::hasC = true, AnWeb::hasFt = false;
bool AnWeb::hasPy_s = false, AnWeb::hasC_s = false, AnWeb::hasFt_s = false;

void AnWeb::Insert(AnScript* scr, Vec2 pos) {
	AnNode* nd;
	//if (scr->type == AN_SCRTYPE::PYTHON)
		//nd = new PyNode((PyScript*)scr);
	if (scr->type == AN_SCRTYPE::PYTHON)
		nd = new PyNode((PyScript*)scr);
	else
		abort();
	Insert(nd, pos);
}

void AnWeb::Insert(AnNode* node, Vec2 pos) {
	nodes.push_back(node);
	nodes.back()->pos = pos;
}

void AnWeb::Init() {
	Insert(new Node_Inputs());
	Insert(new Node_Inputs_ActPar());
}

void AnWeb::Update() {
#ifndef IS_ANSERVER
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
#endif
}

void AnWeb::Draw() {
#ifndef IS_ANSERVER
	AnNode::width = 220;
	Engine::DrawQuad(AnBrowse::expandPos, 0.0f, Display::width - AnBrowse::expandPos - AnOps::expandPos, Display::height - 18.0f, white(0.8f, 0.05f));
	Engine::BeginStencil(AnBrowse::expandPos, 0.0f, Display::width - AnBrowse::expandPos - AnOps::expandPos, Display::height - 18.0f);
	byte ms = Input::mouse0State;
	if (executing) {
		Input::mouse0 = false;
		Input::mouse0State = 0;
	}
	AnWeb::selPreClear = true;
	Vec2 poss(AnBrowse::expandPos + 10 - scrollPos, 100);
	float maxoff = 220, offy = -5;
	maxScroll = 10;
	int ns = nodes.size(), i = 0, iter = -1;
	bool iterTile = false, iterTileTop = false;
	for (auto n : nodes) {
		if (!n->canTile) {
			poss.x += maxoff + 20;
			maxScroll += maxoff + 20;
			poss.y = 100;
			maxoff = 220;
			if (selScript) {
				if (Engine::Button(poss.x, 100, maxoff, 30, white(0.3f, 0.05f)) == MOUSE_RELEASE) {
					iter = i - 1;
					iterTile = true;
					iterTileTop = true;
				}
				UI::Texture(poss.x + 95, 100, 30, 30, Icons::expand, white(0.2f));
				poss.y = 133;
			}
		}
		else {
			poss.y += offy + 5;
		}
		n->pos = poss;
		auto o = n->DrawConn();
		//maxoff = max(maxoff, o.x);
		offy = o.y;
		if (selScript) {
			if (Engine::Button(poss.x, poss.y + offy + 3, maxoff, 30, white(0.3f, 0.05f)) == MOUSE_RELEASE) {
				iter = i;
				iterTile = true;
				iterTileTop = false;
			}
			UI::Texture(poss.x - 15 + maxoff / 2, poss.y + offy + 3, 30, 30, Icons::expand, white(0.2f));
			offy += 31;
			if ((ns == i + 1) || !nodes[i + 1]->canTile) {
				if (Engine::Button(poss.x + maxoff + 10, 100, 30, 200, white(0.3f, 0.05f)) == MOUSE_RELEASE) {
					iter = i;
					iterTile = false;
				}
				UI::Texture(poss.x + maxoff + 10, 100 + 85, 30, 30, Icons::expand, white(0.2f));
				maxoff += 30;
			}
		}
		i++;
	}
	maxScroll += maxoff + (selScript ? 20 : 10);
	for (auto n : nodes) {
		n->Draw();
	}
	if (Input::mouse0State == MOUSE_UP && selPreClear) selConnNode = nullptr;

	float canScroll = max(maxScroll - (Display::width - AnBrowse::expandPos - AnOps::expandPos), 0.0f);
	//if (Input::KeyHold(Key_RightArrow)) scrollPos += 1000 * Time::delta;
	//if (Input::KeyHold(Key_LeftArrow)) scrollPos -= 1000 * Time::delta;
	scrollPos = Clamp(scrollPos - Input::mouseScroll * 1000 * Time::delta, 0.0f, canScroll);

	Input::mouse0State = ms;
	Input::mouse0 = (Input::mouse0State == 1) || (Input::mouse0State == 2);
	Engine::EndStencil();

	if (Input::mouse0State == MOUSE_UP) {
		if (selScript) {
			if (iter >= 0) {
				AnNode* pn;
				if ((uintptr_t)selScript > 1) {
					switch (selScript->type) {
					case AN_SCRTYPE::PYTHON:
						pn = new PyNode((PyScript*)selScript);
						break;
					case AN_SCRTYPE::C:
						pn = new CNode((CScript*)selScript);
						break;
					default:
						abort();
					}
				}
				else {
					switch (selSpNode) {
					case AN_NODE_MISC::PLOT:
						pn = new Node_Plot();
						break;
					case AN_NODE_MISC::VOLUME:
						pn = new Node_Volume();
						break;
					default:
						abort();
					}
				}
				if (iterTile) {
					if (iterTileTop) nodes[iter + 1]->canTile = true;
					else pn->canTile = true;
				}
				nodes.insert(nodes.begin() + iter + 1, pn);
			}
			selScript = nullptr;
		}
		else {
			for (auto nn = nodes.begin(); nn != nodes.end(); nn++) {
				auto& n = *nn;
				if (n->op == ANNODE_OP::REMOVE) {
					if ((nn + 1) != nodes.end()) {
						if (!n->canTile && (*(nn + 1))->canTile)
							(*(nn + 1))->canTile = false;
					}
					for (uint i = 0; i < n->inputR.size(); i++) {
						if (n->inputR[i].first) n->inputR[i].first->outputR[n->inputR[i].second].first = nullptr;
					}
					for (uint i = 0; i < n->outputR.size(); i++) {
						if (n->outputR[i].first) n->outputR[i].first->inputR[n->outputR[i].second].first = nullptr;
					}
					delete(n);
					nodes.erase(nn);
					break;
				}
			}
		}
	}

	AnBrowse::Draw();
	AnOps::Draw();

	if (Input::KeyDown(Key_Escape)) {
		if (selScript) selScript = nullptr;
		else {
			drawFull = false;
			AnBrowse::expandPos = AnOps::expandPos = 0;
		}
	}
	if (selScript) {
		Texture* icon = 0;
		if ((uintptr_t)selScript == 1)
			icon = Icons::lightning;
		else if (selScript->type == AN_SCRTYPE::C)
			icon = Icons::lang_c;
		else if (selScript->type == AN_SCRTYPE::PYTHON)
			icon = Icons::lang_py;
		UI::Texture(Input::mousePos.x - 16, Input::mousePos.y - 16, 32, 32, icon, white(0.3f));
	}

	if (Engine::Button(Display::width - 71.0f, 1.0f, 70.0f, 16.0f, white(1, 0.4f), "Done", 12.0f, AnNode::font, white(), true) == MOUSE_RELEASE) {
		drawFull = false;
		AnBrowse::expandPos = AnOps::expandPos = 0;
	}
	
	if (Engine::Button(200, 1, 70.0f, 16.0f, white(1, 0.4f), "Save", 12.0f, AnNode::font, white(), true) == MOUSE_RELEASE)
		Save(IO::path + "/nodes/rdf.anl");

	if (Engine::Button(275, 1, 70, 16, white(1, executing ? 0.2f : 0.4f), "Run", 12, AnNode::font, white(), true) == MOUSE_RELEASE) {

	}
	if (Engine::Button(350, 1, 107, 16, white(1, executing ? 0.2f : 0.4f), "Run All", 12, AnNode::font, white(), true) == MOUSE_RELEASE) {
		AnWeb::Execute();
	}
	UI::Texture(275, 1, 16, 16, Icons::play);
	UI::Texture(350, 1, 16, 16, Icons::playall);
#endif
}

void AnWeb::DrawSide() {
#ifndef IS_ANSERVER
	Engine::DrawQuad(Display::width - expandPos, 0.0f, 180.0f, Display::height - 18.0f, white(0.9f, 0.15f));
	if (expanded) {
		float w = 180;
		AnNode::width = w - 2;
		UI::Label(Display::width - expandPos + 5, 1, 12, "Analysis", AnNode::font, white());

		if (Engine::Button(Display::width - expandPos + 109, 1, 70, 16, white(1, 0.4f), "Edit", 12, AnNode::font, white(), true) == MOUSE_RELEASE)
			drawFull = true;

		if (Engine::Button(Display::width - expandPos + 1, 18, 70, 16, white(1, executing ? 0.2f : 0.4f), "Run", 12, AnNode::font, white(), true) == MOUSE_RELEASE) {

		}
		if (Engine::Button(Display::width - expandPos + 72, 18, 107, 16, white(1, executing ? 0.2f : 0.4f), "Run All", 12, AnNode::font, white(), true) == MOUSE_RELEASE) {
			AnWeb::Execute();
		}
		UI::Texture(Display::width - expandPos + 1, 18, 16, 16, Icons::play);
		UI::Texture(Display::width - expandPos + 72, 18, 16, 16, Icons::playall);

		//Engine::BeginStencil(Display::width - w, 0, 150, Display::height);
		Vec2 poss(Display::width - expandPos + 1, 35);
		for (auto n : nodes) {
			n->pos = poss;
			poss.y += n->DrawSide();
		}
		//Engine::EndStencil();
		Engine::DrawQuad(Display::width - expandPos - 16.0f, Display::height - 34.0f, 16.0f, 16.0f, white(0.9f, 0.15f));
		if ((!UI::editingText && Input::KeyUp(Key_A)) || Engine::Button(Display::width - expandPos - 16.0f, Display::height - 34.0f, 16.0f, 16.0f, Icons::collapse, white(0.8f), white(), white(0.5f)) == MOUSE_RELEASE)
			expanded = false;
		expandPos = Clamp(expandPos + 1500 * Time::delta, 0.0f, 180.0f);
	}
	else {
		Engine::DrawQuad(Display::width - expandPos, 0.0f, expandPos, Display::height - 18.0f, white(0.9f, 0.15f));
		if ((!UI::editingText && Input::KeyUp(Key_A)) || Engine::Button(Display::width - expandPos - 110.0f, Display::height - 34.0f, 110.0f, 16.0f, white(0.9f, 0.15f), white(1, 0.15f), white(1, 0.05f)) == MOUSE_RELEASE)
			expanded = true;
		UI::Texture(Display::width - expandPos - 110.0f, Display::height - 34.0f, 16.0f, 16.0f, Icons::expand);
		UI::Label(Display::width - expandPos - 92.0f, Display::height - 33.0f, 12.0f, "Analysis (A)", AnNode::font, white());
		expandPos = Clamp(expandPos - 1500 * Time::delta, 2.0f, 180.0f);
	}
#endif
}

void AnWeb::DrawScene() {
#ifndef IS_ANSERVER
	for (auto& n : nodes) n->DrawScene();
#endif
}

void AnWeb::Execute() {
	if (!executing) {
		executing = true;
		if (execThread) delete(execThread);
		execThread = new std::thread(DoExecute);
	}
}

void AnWeb::DoExecute() {
	for (auto n : nodes) n->Execute();
	executing = false;
}

#define sp << " "
#define nl << "\n"
void AnWeb::Save(const string& s) {
	std::ofstream strm(s, std::ios::binary);
	strm << nodes.size() nl;
	uint i = 0;
	for (auto n : nodes) {
		n->id = i++;
		n->Save(strm);
	}
	strm.close();
	SaveIn();
}

void AnWeb::SaveIn() {
	string path = IO::path + "/nodes/__tmp__/";
	if (!IO::HasDirectory(path)) IO::MakeDirectory(path);
	path += "in/";
	if (!IO::HasDirectory(path)) IO::MakeDirectory(path);
	for (auto n : nodes) {
		n->SaveIn(path);
	}
}

void AnWeb::Load(const string& s) {
	std::ifstream strm(s, std::ios::binary);
	if (!strm.is_open())
		return;
	uint sz;
	strm >> sz;
	nodes.resize(sz);
	int t;
	string nm;
	for (auto a = 0; a < sz; a++) {
		strm >> t >> nm;
		switch (t) {
		case 0:
			if (nm == ".in") nodes[a] = new Node_Inputs();
			else if (nm == ".insel") nodes[a] = new Node_Inputs_ActPar();
			else if (nm == ".Vol") nodes[a] = new Node_Volume();
			else if (nm == ".Plot") nodes[a] = new Node_Plot();
			break;
		case 1:
			nodes[a] = new CNode(CScript::allScrs[nm]);
			break;
		case 2:
			nodes[a] = new PyNode(PyScript::allScrs[nm]);
			break;
		default:
			abort();
		}
		nodes[a]->Load(strm);
	}
}

void AnWeb::LoadIn() {
	string path = IO::path + "/nodes/__tmp__/in/";
	for (auto n : nodes) {
		n->LoadIn(path);
	}
}