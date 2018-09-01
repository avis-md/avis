#include "anweb.h"
#ifndef IS_ANSERVER
#include "anops.h"
#include "errorview.h"
#include "ui/icons.h"
#include "md/Particles.h"
#include "vis/pargraphics.h"
#endif

//#define NO_REDIR_LOG

AnNode* AnWeb::selConnNode = nullptr;
uint AnWeb::selConnId = 0;
bool AnWeb::selConnIdIsOut = false, AnWeb::selPreClear = false;
AnScript* AnWeb::selScript = nullptr;
byte AnWeb::selSpNode = 0;

string AnWeb::activeFile = "";
std::vector<AnNode*> AnWeb::nodes;

bool AnWeb::drawFull = false, AnWeb::expanded = true, AnWeb::executing = false, AnWeb::apply = false;
float AnWeb::maxScroll, AnWeb::scrollPos = 0, AnWeb::expandPos = 0;

std::thread* AnWeb::execThread = nullptr;
AnNode* AnWeb::execNode = nullptr;

bool AnWeb::hasPy = false, AnWeb::hasC = true, AnWeb::hasFt = true;
bool AnWeb::hasPy_s = false, AnWeb::hasC_s = false, AnWeb::hasFt_s = false;

void AnWeb::Init() {
	Insert(new Node_Inputs());
	for (int a = 0; a < 10; a++) {
		AnBrowse::mscFdExpanded[a] = true;
	}
	ChokoLait::focusFuncs.push_back(CheckChanges);
}

void AnWeb::Insert(AnScript* scr, Vec2 pos) {
	AnNode* nd;
	if (scr->type == AN_SCRTYPE::PYTHON)
		nd = new PyNode((PyScript*)scr);
	else {
		Debug::Error("AnWeb::Insert", "Invalid type!");
		return;
	}
	Insert(nd, pos);
}

void AnWeb::Insert(AnNode* node, Vec2 pos) {
	nodes.push_back(node);
	nodes.back()->pos = pos;
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
	if (!executing && execThread) { //do I need this?
		if (execThread->joinable()) execThread->join();
	}
	if (apply) {
		apply = false;
		auto frm = Particles::anim.activeFrame;
		Particles::anim.activeFrame = -1;
		Particles::SetFrame(frm);
	}

	if (Input::KeyDown(Key_P)) Save(IO::path + "/nodes/test.web");
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
				AnNode* pn = 0;
				if ((uintptr_t)selScript > 1) {
					switch (selScript->type) {
					case AN_SCRTYPE::PYTHON:
						pn = new PyNode((PyScript*)selScript);
						break;
					case AN_SCRTYPE::C:
						pn = new CNode((CScript*)selScript);
						break;
					case AN_SCRTYPE::FORTRAN:
						pn = new FNode((FScript*)selScript);
						break;
					default:
						Debug::Error("AnWeb::Draw", "Unhandled script type: " + std::to_string((int)selScript->type));
						break;
					}
				}
				else {
#define SW(nm, scr) case (byte)AN_NODE_ ## nm: pn = new scr(); break
					switch (selSpNode) {
						SW(SCN::OCAM, Node_Camera_Out);

						SW(IN::SELPAR, Node_Inputs_SelPar);

						SW(MOD::RECOL, Node_Recolor);
						SW(MOD::RECOLA, Node_Recolor_All);

						SW(MOD::PARAM, Node_SetParam);

						SW(GEN::BOND, Node_AddBond);
						SW(GEN::VOL, Node_Volume);
						SW(GEN::TRJ, Node_TraceTrj);

						SW(MISC::PLOT, Node_Plot);
						SW(MISC::SRNG, Node_ShowRange);
					default:
						Debug::Error("AnWeb::Draw", "Unhandled node type: " + std::to_string((int)selSpNode));
						return;
					}
#undef SW
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

	ErrorView::Draw();

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
		else
			icon = Icons::lang_ft;
		UI::Texture(Input::mousePos.x - 16, Input::mousePos.y - 16, 32, 32, icon, white(0.3f));
	}

	if (Engine::Button(Display::width - 71.0f, 1.0f, 70.0f, 16.0f, white(1, 0.4f), "Done", 12.0f, white(), true) == MOUSE_RELEASE) {
		drawFull = false;
		AnBrowse::expandPos = AnOps::expandPos = 0;
	}
	
	if (Engine::Button(200, 1, 70.0f, 16.0f, white(1, 0.4f), "Save", 12.0f, white(), true) == MOUSE_RELEASE)
		Save(IO::path + "/nodes/rdf.anl");

	if (Engine::Button(275, 1, 70, 16, white(1, executing ? 0.2f : 0.4f), "Run", 12, white(), true) == MOUSE_RELEASE) {

	}
	bool canexec = (!AnOps::remote || (AnOps::connectStatus == 255));
	if (Engine::Button(350, 1, 107, 16, white(1, (!canexec || executing) ? 0.2f : 0.4f), "Run All", 12, white(), true) == MOUSE_RELEASE) {
		if (canexec) AnWeb::Execute();
	}
	UI::Texture(275, 1, 16, 16, Icons::play);
	UI::Texture(350, 1, 16, 16, Icons::playall);
#endif
}

void AnWeb::DrawSide() {
#ifndef IS_ANSERVER
	Engine::DrawQuad(Display::width - expandPos, 18, 180.0f, Display::height - 18.0f, white(0.9f, 0.15f));
	if (expanded) {
		float w = 180;
		AnNode::width = w - 2;
		UI::Label(Display::width - expandPos + 5, 20, 12, "Analysis", white());

		if (Engine::Button(Display::width - expandPos + 109, 20, 70, 16, white(1, 0.4f), "Edit", 12, white(), true) == MOUSE_RELEASE)
			drawFull = true;

		if (Engine::Button(Display::width - expandPos + 1, 38, 70, 16, white(1, executing ? 0.2f : 0.4f), "Run", 12, white(), true) == MOUSE_RELEASE) {

		}
		if (Engine::Button(Display::width - expandPos + 72, 38, 107, 16, white(1, executing ? 0.2f : 0.4f), "Run All", 12, white(), true) == MOUSE_RELEASE) {
			AnWeb::Execute();
		}
		UI::Texture(Display::width - expandPos + 1, 38, 16, 16, Icons::play);
		UI::Texture(Display::width - expandPos + 72, 38, 16, 16, Icons::playall);

		//Engine::BeginStencil(Display::width - w, 0, 150, Display::height);
		Vec2 poss(Display::width - expandPos + 1, 17 * 3 + 4);
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
		UI::Label(Display::width - expandPos - 92.0f, Display::height - 33.0f, 12.0f, "Analysis (A)", white());
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
		if (execThread) {
			if (execThread->joinable()) execThread->join();
			delete(execThread);
		}
#ifndef IS_ANSERVER
		if (AnOps::remote) {
			Save(activeFile);
			execThread = new std::thread(DoExecute_Srv);
		}
		else
#endif
			execThread = new std::thread(DoExecute);
	}
}

void AnWeb::DoExecute() {
#ifndef NO_REDIR_LOG
	IO::StartReadStdio(IO::path + "/nodes/__tmpstd", OnExecLog);
	execNode = (AnNode*)2;
	std::cout << "__start__\n";
	std::cerr << "__start__\n";
	while (execNode) {}
#endif
	for (auto n : nodes) {
		n->log.clear();
	}
	for (auto n : nodes) {
		execNode = n;
		n->executing = true;
		try {
			n->Execute();
		}
		catch (char* e) {
			n->log.push_back(std::pair<byte, string>(2, string(e)));
		}
		n->executing = false;
#ifndef NO_REDIR_LOG
		std::cout << "\n___123___\n";
		IO::FlushStdio();
		while (execNode) {}
	}
	IO::StopReadStdio();
#else
	}
#endif
	execNode = nullptr;
	executing = false;
	apply = true;
}

void AnWeb::OnExecLog(string s, bool e) {
	if (execNode) {
		if (s == "___123___") {
			if (execNode->log.back().second == "")
				execNode->log.pop_back();
			execNode = nullptr;
		}
		else if ((uintptr_t)execNode <= 2) {
			(*(uintptr_t*)&execNode)--;
		}
		else execNode->log.push_back(std::pair<byte, string>(e ? 1 : 0, s));
	}
}

void AnWeb::DoExecute_Srv() {
#ifndef IS_ANSERVER
	AnOps::message = "checking for lock";
	if (AnOps::ssh.HasFile(AnOps::path + "/.lock")) {
		Debug::Warning("AnWeb", "An existing session is running!");
		executing = false;
		return;
	}
	AnOps::message = "cleaning old files";
	AnOps::ssh.Write("cd ser; rm -f ./*; cd in; rm -f ./*; cd ../out; rm -f ./*; cd " + AnOps::path);
	AnOps::message = "syncing files";
	AnOps::ssh.SendFile(activeFile, AnOps::path + "/ser/web.anl");
	AnOps::SendIn();
	AnOps::ssh.Write("chmod +r ser/web.anl");
	AnOps::message = "running";
	AnOps::ssh.Write("./mdvis_ansrv; echo '$%''%$'");
	AnOps::ssh.WaitFor("$%%$", 200);
	AnOps::RecvOut();
	LoadOut();
	AnOps::message = "finished";
	executing = false;
	apply = true;
#endif
}

#define sp << " "
#define nl << "\n"
#define wrs(s) _StreamWrite(s.c_str(), &strm, s.size() + 1);
void AnWeb::Save(const string& s) {
	std::ofstream strm(s, std::ios::binary);
	auto sz = (uint32_t)nodes.size();
	_StreamWrite(&sz, &strm, 4);
	int i = 0;
	for (auto n : nodes) {
		n->id = i++;
		_StreamWrite(&n->script->type, &strm, 1);
		wrs(n->script->name);
		strm.write((n->canTile ? "\x01" : "\x00"), 1);
		n->SaveConn();
		sz = n->_connInfo.size();
		_StreamWrite(&sz, &strm, 2);
		for (auto& c : n->_connInfo) {
			strm.write((c.cond ? "\x01" : "\x00"), 1);
			if (c.cond) {
				wrs(c.mynm);
				wrs(c.mytp);
				_StreamWrite(&c.tar->id, &strm, 1);
				wrs(c.tarnm);
				wrs(c.tartp);
			}
		}
	}
	strm.close();
	SaveIn();
}

void AnWeb::SaveIn() {
	string path = IO::path + "/nodes/__tmp__/";
	if (!IO::HasDirectory(path)) IO::MakeDirectory(path);
	path += "in/";
	if (!IO::HasDirectory(path)) IO::MakeDirectory(path);
	else {
		auto fls = IO::GetFiles(path);
		for (auto& f : fls) {
			remove((path + "/" + f).c_str());
		}
	}
	for (auto n : nodes) {
		n->SaveIn(path);
	}
}

void AnWeb::SaveOut() {
#ifdef IS_ANSERVER
	string path = IO::path + "/ser/";
#else
	string path = IO::path + "/nodes/__tmp__/";
#endif
	if (!IO::HasDirectory(path)) IO::MakeDirectory(path);
	path += "out/";
	if (!IO::HasDirectory(path)) IO::MakeDirectory(path);
	else {
		auto fls = IO::GetFiles(path);
		for (auto& f : fls) {
			remove((path + "/" + f).c_str());
		}
	}
	for (auto n : nodes) {
		n->SaveOut(path);
	}
}

byte GB(std::istream& strm) {
	byte b;
	strm.read((char*)&b, 1);
	return b;
}

void AnWeb::Load(const string& s) {
	std::ifstream strm(s, std::ios::binary);
	if (!strm.is_open()) {
		Debug::Warning("AnWeb", "Cannot open save file!");
		return;
	}
	uint32_t sz;
	_Strm2Val(strm, sz);
	nodes.resize(sz);
	AN_SCRTYPE tp;
	string nm;
	for (uint a = 0; a < sz; a++) {
		auto& n = nodes[a];
		tp = (AN_SCRTYPE)GB(strm);
		std::getline(strm, nm, char0);
		switch (tp) {
		case AN_SCRTYPE::NONE:
#define ND(scr) if (nm == scr::sig) n = new scr(); else
			ND(Node_Inputs)
			ND(Node_Inputs_ActPar)
			ND(Node_Inputs_SelPar)
			ND(Node_AddBond)
			//ND(Node_AddVolume)
			ND(Node_TraceTrj)
			ND(Node_Camera_Out)
			ND(Node_Recolor)
			ND(Node_Recolor_All)
			ND(Node_SetParam)
			ND(Node_Volume)
			ND(Node_Plot)
			ND(Node_ShowRange)
			Debug::Warning("AnWeb::Load", "Unknown node name: " + nm);
#undef ND
			break;
		case AN_SCRTYPE::C:
			n = new CNode(CScript::allScrs[nm]);
			break;
		case AN_SCRTYPE::PYTHON:
			n = new PyNode(PyScript::allScrs[nm]);
			break;
		case AN_SCRTYPE::FORTRAN:
			n = new FNode(FScript::allScrs[nm]);
			break;
		default:
			Debug::Warning("AnWeb::Load", "Unknown node type: " + std::to_string((byte)tp));
			break;
		}
		n->canTile = !!GB(strm);
		uint16_t csz;
		_Strm2Val(strm, csz);
		n->_connInfo.resize(csz);
		for (uint16_t q = 0; q < csz; q++) {
			auto& c = n->_connInfo[q];
			c.cond = !!GB(strm);
			if (c.cond) {
				std::getline(strm, c.mynm, char0);
				std::getline(strm, c.mytp, char0);
				c.tar = nodes[GB(strm)];
				std::getline(strm, c.tarnm, char0);
				std::getline(strm, c.tartp, char0);
			}
		}
	}
	Reconn();
	activeFile = s;
}

void AnWeb::LoadIn() {
#ifdef IS_ANSERVER
	string path = IO::path + "/ser/in/";
#else
	string path = IO::path + "/nodes/__tmp__/in/";
#endif
	for (auto n : nodes) {
		n->LoadIn(path);
	}
}

void AnWeb::LoadOut() {
	string path = IO::path + "/nodes/__tmp__/out/";
	for (auto n : nodes) {
		n->LoadOut(path);
	}
}

void AnWeb::CheckChanges() {
	SaveConn();
	AnBrowse::Refresh();
	ClearConn();
	Reconn();
}

void AnWeb::SaveConn() {
	for (auto n : nodes) {
		n->SaveConn();
	}
}

void AnWeb::ClearConn() {
	for (auto n : nodes) {
		n->ClearConn();
	}
}

void AnWeb::Reconn() {
	for (auto n : nodes) {
		n->Reconn();
	}
}

void AnWeb::OnSceneUpdate() {
	for (auto n : nodes) {
		n->OnSceneUpdate();
	}
}

void AnWeb::OnAnimFrame() {
	for (auto n : nodes) {
		n->OnAnimFrame();
	}
}