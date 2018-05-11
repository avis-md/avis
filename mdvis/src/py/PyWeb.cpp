#include "PyWeb.h"
#include "ui/icons.h"
#include "md/Particles.h"
#include "vis/pargraphics.h"

PyNode* PyWeb::selConnNode = nullptr;
uint PyWeb::selConnId = 0;
bool PyWeb::selConnIdIsOut = false, PyWeb::selPreClear = false;
PyScript* PyWeb::selScript = nullptr;

std::vector<PyNode*> PyWeb::nodes;

bool PyWeb::drawFull = false, PyWeb::expanded = true, PyWeb::executing = false;
float PyWeb::maxScroll, PyWeb::scrollPos = 0, PyWeb::expandPos = 150.0f;

std::thread* PyWeb::execThread = nullptr;

void PyWeb::Insert(PyScript* scr, Vec2 pos) {
	Insert(new PyNode(scr), pos);
}

void PyWeb::Insert(PyNode* node, Vec2 pos) {
	nodes.push_back(node);
	nodes.back()->pos = pos;
}

void PyWeb::Init() {
	auto scr = PyBrowse::folder.scripts[1];
	auto scr2 = PyBrowse::folder.scripts[0];
	
	Insert(new PyNode_Inputs());
	Insert(new PyNode_Inputs_ActPar());
	Insert(scr);
	Insert(scr2);
	Insert(new PyNode_Plot());
	nodes[2]->outputR[0] = nodes[3];
	nodes[3]->inputR[0] = nodes[2];
	nodes[3]->outputR[1] = nodes[4];
	nodes[4]->inputR[0] = nodes[3];
	nodes[4]->inputV[0].second = 1;
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
	Engine::DrawQuad(PyBrowse::expandPos, 0.0f, (float)Display::width, Display::height - 18.0f, white(0.8f, 0.05f));
	Engine::BeginStencil(PyBrowse::expandPos, 0.0f, (float)Display::width, Display::height - 18.0f);
	byte ms = Input::mouse0State;
	if (executing) {
		Input::mouse0 = false;
		Input::mouse0State = 0;
	}
	PyWeb::selPreClear = true;
	Vec2 poss(PyBrowse::expandPos + 10 - scrollPos, 100);
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

	float canScroll = max(maxScroll - (Display::width - PyBrowse::expandPos), 0.0f);
	//if (Input::KeyHold(Key_RightArrow)) scrollPos += 1000 * Time::delta;
	//if (Input::KeyHold(Key_LeftArrow)) scrollPos -= 1000 * Time::delta;
	scrollPos = Clamp(scrollPos - Input::mouseScroll * 1000 * Time::delta, 0.0f, canScroll);

	Input::mouse0State = ms;
	Input::mouse0 = (Input::mouse0State == 1) || (Input::mouse0State == 2);
	Engine::EndStencil();

	if (Input::mouse0State == MOUSE_UP) {
		if (selScript) {
			if (iter >= 0) {
				auto pn = new PyNode(selScript);
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
				if (n->op == PYNODE_OP::REMOVE) {
					if ((nn + 1) != nodes.end()) {
						if (!n->canTile && (*(nn + 1))->canTile)
							(*(nn + 1))->canTile = false;
					}
					for (uint i = 0; i < n->inputR.size(); i++) {
						if (n->inputR[i]) n->inputR[i]->outputR[n->inputV[i].second] = nullptr;
					}
					for (uint i = 0; i < n->outputR.size(); i++) {
						if (n->outputR[i]) n->outputR[i]->inputR[n->outputV[i].second] = nullptr;
					}
					delete(n);
					nodes.erase(nn);
					break;
				}
			}
		}
	}

	PyBrowse::Draw();

	if (Input::KeyDown(Key_Escape)) {
		if (selScript) selScript = nullptr;
		else drawFull = false;
	}
	if (selScript) UI::Texture(Input::mousePos.x - 16, Input::mousePos.y - 16, 32, 32, Icons::python, white(0.3f));
	
	if (Engine::Button(Display::width - 71.0f, 1.0f, 70.0f, 16.0f, white(1, 0.4f), "Done", 12.0f, PyNode::font, white(), true) == MOUSE_RELEASE)
		drawFull = false;
}

void PyWeb::DrawSide() {
	Engine::DrawQuad(Display::width - expandPos, 0.0f, 180.0f, Display::height - 18.0f, white(0.9f, 0.15f));
	if (expanded) {
		float w = 180;
		PyNode::width = w - 2;
		UI::Label(Display::width - expandPos + 5, 1, 12, "Analysis", PyNode::font, white());

		if (Engine::Button(Display::width - expandPos + 109, 1, 70, 16, white(1, 0.4f), "Edit", 12, PyNode::font, white(), true) == MOUSE_RELEASE)
			drawFull = true;

		if (Engine::Button(Display::width - expandPos + 1, 18, 70, 16, white(1, executing ? 0.2f : 0.4f), "Run", 12, PyNode::font, white(), true) == MOUSE_RELEASE) {

		}
		if (Engine::Button(Display::width - expandPos + 72, 18, 107, 16, white(1, executing ? 0.2f : 0.4f), "Run All", 12, PyNode::font, white(), true) == MOUSE_RELEASE) {
			PyWeb::Execute();
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
		if (Engine::Button(Display::width - expandPos - 16.0f, Display::height - 34.0f, 16.0f, 16.0f, Icons::collapse, white(0.8f), white(), white(0.5f)) == MOUSE_RELEASE)
			expanded = false;
		expandPos = Clamp(expandPos + 1500 * Time::delta, 0.0f, 180.0f);
	}
	else {
		Engine::DrawQuad(Display::width - expandPos, 0.0f, expandPos, Display::height - 18.0f, white(0.9f, 0.15f));
		if (Engine::Button(Display::width - expandPos - 110.0f, Display::height - 34.0f, 110.0f, 16.0f, white(0.9f, 0.15f), white(1, 0.15f), white(1, 0.05f)) == MOUSE_RELEASE)
			expanded = true;
		UI::Texture(Display::width - expandPos - 110.0f, Display::height - 34.0f, 16.0f, 16.0f, Icons::expand);
		UI::Label(Display::width - expandPos - 92.0f, Display::height - 33.0f, 12.0f, "Analysis", PyNode::font, white());
		expandPos = Clamp(expandPos - 1500*Time::delta, 2.0f, 180.0f);
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