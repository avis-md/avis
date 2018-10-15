#include "anweb.h"
#ifndef IS_ANSERVER
#include "anops.h"
#include "errorview.h"
#include "ui/localizer.h"
#include "ui/icons.h"
#include "md/Particles.h"
#include "md/parloader.h"
#include "vis/pargraphics.h"
#include "vis/system.h"
#endif

#define NO_REDIR_LOG
//#define VERBOSE

bool AnWeb::lazyLoad = true;

AnNode* AnWeb::selConnNode = nullptr;
uint AnWeb::selConnId = 0;
bool AnWeb::selConnIdIsOut = false, AnWeb::selPreClear = false;
AnScript* AnWeb::selScript = nullptr;
byte AnWeb::selSpNode = 0;

std::string AnWeb::activeFile = "";
std::vector<AnNode*> AnWeb::nodes;

bool AnWeb::drawFull = false, AnWeb::expanded = true;
bool AnWeb::executing = false;
bool AnWeb::apply = false;
float AnWeb::maxScroll, AnWeb::scrollPos = 0, AnWeb::expandPos = 0;
int AnWeb::execFrame;
float AnWeb::drawLerp;

std::thread* AnWeb::execThread = nullptr;
AnNode* AnWeb::execNode = nullptr;

bool AnWeb::hasPy = false, AnWeb::hasC = false, AnWeb::hasFt = false;
bool AnWeb::hasPy_s = false, AnWeb::hasC_s = false, AnWeb::hasFt_s = false;

void AnWeb::Init() {
	Insert(new Node_Inputs());
	Insert(new Node_Info());
	for (int a = 0; a < 10; a++) {
		AnBrowse::mscFdExpanded[a] = true;
	}
	ChokoLait::focusFuncs.push_back(CheckChanges);
}

void AnWeb::Clear() {
	for (auto n : nodes) {
		delete n;
	}
	nodes.clear();
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
		auto frm = Particles::anim.currentFrame;
		Particles::anim.currentFrame = -1;
		Particles::SetFrame(frm);
	}

	if (Input::KeyDown(Key_P)) Save(IO::path + "nodes/test.web");
#endif
}

void AnWeb::Draw() {
#ifndef IS_ANSERVER
	AnNode::width = 220;
	UI::Quad(AnBrowse::expandPos, 0.f, Display::width - AnBrowse::expandPos - AnOps::expandPos, Display::height - 18.f, white(0.8f * drawLerp, 0.05f));
	Engine::BeginStencil(AnBrowse::expandPos, 0.f, Display::width - AnBrowse::expandPos - AnOps::expandPos, Display::height - 18.f);
	byte ms = Input::mouse0State;
	if (executing) {
		Input::mouse0 = false;
		Input::mouse0State = 0;
	}
	AnWeb::selPreClear = true;
	Vec2 poss(AnBrowse::expandPos + 10 - scrollPos, 100);
	float maxoff = 220, offy = -5;
	maxScroll = 10;
	int ns = (int)nodes.size(), i = 0, iter = -1;
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

	float canScroll = max(maxScroll - (Display::width - AnBrowse::expandPos - AnOps::expandPos), 0.f);
	//if (Input::KeyHold(Key_RightArrow)) scrollPos += 1000 * Time::delta;
	//if (Input::KeyHold(Key_LeftArrow)) scrollPos -= 1000 * Time::delta;
	scrollPos = Clamp(scrollPos - Input::mouseScroll * 1000 * Time::delta, 0.f, canScroll);

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
						pn = new PyNode(dynamic_cast<PyScript*>(selScript));
						break;
					case AN_SCRTYPE::C:
						pn = new CNode(dynamic_cast<CScript*>(selScript));
						break;
					case AN_SCRTYPE::FORTRAN:
						pn = new FNode(dynamic_cast<FScript*>(selScript));
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

						SW(IN::PARS, Node_Inputs);
						SW(IN::PINFO, Node_Info);

						SW(MOD::PARAM, Node_SetParam);

						SW(GEN::BOND, Node_AddBond);
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
				else pn->canTile = false;
				nodes.insert(nodes.begin() + iter + 1, pn);
				for (size_t a = 0; a < nodes.size(); a++)
					nodes[a]->id = (uint)a;
			}
			selScript = nullptr;
		}
		else {
			for (auto nn = nodes.begin() + 1; nn != nodes.end(); nn++) {
				auto& n = *nn;
				if (n->op == ANNODE_OP::REMOVE) {
					if ((nn + 1) != nodes.end()) {
						if (!n->canTile && (*(nn + 1))->canTile)
							(*(nn + 1))->canTile = false;
					}
					n->ClearConn();
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
		Texture icon = Icons::lang_ft;
		if ((uintptr_t)selScript == 1)
			icon = Icons::lightning;
		else if (selScript->type == AN_SCRTYPE::C)
			icon = Icons::lang_c;
		else if (selScript->type == AN_SCRTYPE::PYTHON)
			icon = Icons::lang_py;
		UI::Texture(Input::mousePos.x - 16, Input::mousePos.y - 16, 32, 32, icon, white(0.3f));
	}

	if (Engine::Button(Display::width - 71.f, 1.f, 70.f, 16.f, white(1, 0.4f), _("Done"), 12.f, white(), true) == MOUSE_RELEASE) {
		drawFull = false;
		AnBrowse::expandPos = AnOps::expandPos = 0;
	}
	
	if (Engine::Button(200, 1, 70.f, 16.f, white(1, 0.4f), _("Save"), 12.f, white(), true) == MOUSE_RELEASE)
		Save(IO::path + "nodes/rdf.anl");

	bool canexec = (!AnOps::remote || (AnOps::connectStatus == 255)) && !executing && !ParLoader::busy && !AnBrowse::busy;
	if (Engine::Button(275, 1, 70, 16, white(1, canexec ? 0.4f : 0.2f), _("Run"), 12, white(), true) == MOUSE_RELEASE) {
		if (canexec) AnWeb::Execute(false);
	}
	if (Engine::Button(350, 1, 107, 16, white(1, canexec ? 0.4f : 0.2f), _("Run All"), 12, white(), true) == MOUSE_RELEASE) {
		if (canexec) AnWeb::Execute(true);
	}
	UI::Texture(275, 1, 16, 16, Icons::play);
	UI::Texture(350, 1, 16, 16, Icons::playall);
	if (drawLerp < 1) drawLerp = min(drawLerp + 10 * Time::delta, 1.f);
#endif
}

void AnWeb::DrawSide() {
#ifndef IS_ANSERVER
	UI::Quad(Display::width - expandPos, 18, 180.f, Display::height - 36.f, white(0.9f, 0.15f));
	if (expanded) {
		float w = 180;
		AnNode::width = w - 2;
		UI::Label(Display::width - expandPos + 5, 20, 12, _("Analysis"), white());

		if (Engine::Button(Display::width - expandPos + 109, 20, 70, 16, white(1, 0.4f), _("Edit"), 12, white(), true) == MOUSE_RELEASE)
			drawFull = true;

		bool canexec = (!AnOps::remote || (AnOps::connectStatus == 255)) && !executing && !ParLoader::busy && !AnBrowse::busy;
		if (Engine::Button(Display::width - expandPos + 1, 38, 70, 16, white(1, canexec ? 0.4f : 0.2f), _("Run"), 12, white(), true) == MOUSE_RELEASE) {
			if (canexec) AnWeb::Execute(false);
		}
		if (!execFrame) {
			if (Engine::Button(Display::width - expandPos + 72, 38, 107, 16, white(1, canexec ? 0.4f : 0.2f), _("Run All"), 12, white(), true) == MOUSE_RELEASE) {
				if (canexec) AnWeb::Execute(true);
			}
		}
		else {
			UI::Quad(Display::width - expandPos + 72, 38, 107, 16, white(1, 0.2f));
			UI::font->Align(ALIGN_TOPCENTER);
			UI::Label(Display::width - expandPos + 72 + 54, 39, 12, std::to_string(execFrame), white(0.8f));
			UI::font->Align(ALIGN_TOPLEFT);
		}
		UI::Texture(Display::width - expandPos + 1, 38, 16, 16, Icons::play);
		UI::Texture(Display::width - expandPos + 72, 38, 16, 16, Icons::playall);

		Vec2 poss(Display::width - expandPos + 1, 17 * 3 + 4);
		for (auto n : nodes) {
			n->pos = poss;
			poss.y += n->DrawSide();
		}
		UI::Quad(Display::width - expandPos - 16.f, Display::height - 34.f, 16.f, 16.f, white(0.9f, 0.15f));
		if ((!UI::editingText && Input::KeyUp(Key_A)) || Engine::Button(Display::width - expandPos - 16.f, Display::height - 34.f, 16.f, 16.f, Icons::collapse, white(0.8f), white(), white(0.5f)) == MOUSE_RELEASE)
			expanded = false;
		expandPos = Clamp(expandPos + 1500 * Time::delta, 0.f, 180.f);
	}
	else {
		UI::Quad(Display::width - expandPos, 0.f, expandPos, Display::height - 18.f, white(0.9f, 0.15f));
		if ((!UI::editingText && Input::KeyUp(Key_A)) || Engine::Button(Display::width - expandPos - 110.f, Display::height - 34.f, 110.f, 16.f, white(0.9f, 0.15f), white(1, 0.15f), white(1, 0.05f)) == MOUSE_RELEASE)
			expanded = true;
		UI::Texture(Display::width - expandPos - 110.f, Display::height - 34.f, 16.f, 16.f, Icons::expand);
		UI::Label(Display::width - expandPos - 92.f, Display::height - 33.f, 12.f, _("Analysis") +" (A)", white());
		expandPos = Clamp(expandPos - 1500 * Time::delta, 2.f, 180.f);
	}
	drawLerp = 0;
#endif
}

void AnWeb::DrawScene() {
#ifndef IS_ANSERVER
	for (auto& n : nodes) n->DrawScene();
#endif
}

void AnWeb::Execute(bool all) {
	if (!Particles::particleSz || executing) return;
	ParGraphics::animate = false;
	if (execThread) {
		if (execThread->joinable()) execThread->join();
		delete(execThread);
	}
	executing = true;
	execFrame = all? 1 : 0;
#ifndef IS_ANSERVER
	if (AnOps::remote) {
		Save(activeFile);
		execThread = new std::thread(DoExecute_Srv);
	}
	else
#endif
		execThread = new std::thread(DoExecute, all);
}

void AnWeb::DoExecute(bool all) {
	Engine::stateLock2.lock();

	for (auto n : nodes) {
		n->log.clear();
	}
	ErrorView::execMsgs.clear();
	
	if (all) {
		auto f = Particles::anim.currentFrame;
		ApplyFrameCount(Particles::anim.frameCount);
		for (uint a = 0; a < Particles::anim.frameCount; a++) {
			execFrame = a+1;
			Particles::anim.Seek(a);
			auto st = Particles::anim.status[a];
			if (st != Particles::AnimData::FRAME_STATUS::LOADED){
				Debug::Warning("AnWeb::Execute", "failed to seek to frame " + std::to_string(a) + "! " + std::to_string((int)st));
				executing = false;
				Engine::stateLock2.unlock();
				Particles::anim.Seek(f);
				execFrame = 0;
				return;
			}
			Node_Inputs::frame = a;
			_DoExecute();
			AnWeb::WriteFrame(a);
		}
		execFrame = 0;
		Particles::anim.Seek(f);
	}
	else {
		RemoveFrames();
		Node_Inputs::frame = Particles::anim.currentFrame;
		_DoExecute();
	}

	executing = false;
	apply = true;

	Engine::stateLock2.unlock();
}

void AnWeb::_DoExecute() {
	char* err = 0;
	static std::string pylog;
	for (auto n : nodes) {
#ifdef VERBOSE
		Debug::Message("AnWeb", "Executing " + n->script->name);
#endif
#ifndef NO_REDIR_LOG
		if (n->script->type == AN_SCRTYPE::PYTHON)
			PyScript::ClearLog();
		else
			IO::RedirectStdio2(IO::path + "nodes/__tmpstd");
#endif
		execNode = n;
		n->executing = true;
		std::exception_ptr cxp;
		try {
			n->Execute();
		}
		catch (const char* e) {
			err = const_cast<char*>(e);
		}
		catch (...) {
			err = (char*)"Unknown error thrown!";
			cxp = std::current_exception();
		}
		n->executing = false;

		if (cxp) {
			try {
				std::rethrow_exception(cxp);
			}
			catch (std::exception& e) {
				static std::string serr = e.what();
				serr += " thrown!";
				err = &serr[0];
			}
			catch (...) {}
		}

#ifndef NO_REDIR_LOG
		if (n->script->type == AN_SCRTYPE::PYTHON) {
			pylog = PyScript::GetLog();
		}
		else {
			IO::RestoreStdio2();
			auto f = std::ifstream(IO::path + "nodes/__tmpstd_o");
			std::string s;
			while (std::getline(f, s)) {
				n->log.push_back(std::pair<byte, std::string>(0, s));
			}
			f.close();
			f.open(IO::path + "nodes/__tmpstd_e");
			while (std::getline(f, s)) {
				n->log.push_back(std::pair<byte, std::string>(2, s));
			}
		}
#endif
		if (n->script->type == AN_SCRTYPE::PYTHON) {
			if (err)
				n->CatchExp(&pylog[0]);
			else {
				auto ss = string_split(pylog, '\n');
				for (auto& s : ss) {
					n->log.push_back(std::pair<byte, std::string>(0, s));
				}
			}
		}
		else if (err) {
			n->CatchExp(err);
			break;
		}
#ifndef NO_REDIR_LOG
		remove((IO::path + "nodes/__tmpstd_o").c_str());
		remove((IO::path + "nodes/__tmpstd_e").c_str());
#endif
#ifdef VERBOSE
		Debug::Message("AnWeb", "Executed " + n->script->name);
#endif
	}
	execNode = nullptr;
}

void AnWeb::OnExecLog(std::string s, bool e) {
	if (execNode) {
		if (s == "___123___") {
			if (execNode->log.back().second == "")
				execNode->log.pop_back();
			execNode = nullptr;
		}
		else if ((uintptr_t)execNode <= 2) {
			(*(uintptr_t*)&execNode)--;
		}
		else execNode->log.push_back(std::pair<byte, std::string>(e ? 1 : 0, s));
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

void AnWeb::ApplyFrameCount(int f) {
	for (auto& n : nodes)
		n->ApplyFrameCount(f);
}

void AnWeb::WriteFrame(uint f) {
	for (auto& n : nodes)
		n->WriteFrame(f);
}

void AnWeb::ReadFrame(uint f) {
	for (auto& n : nodes) {
		if (!n->ReadFrame(f)) return;
	}
}

void AnWeb::RemoveFrames() {
	for (auto& n : nodes) {
		n->RemoveFrames();
	}
}


#define sp << " "
#define nl << "\n"
#define wrs(s) _StreamWrite(s.c_str(), &strm, s.size() + 1);
void AnWeb::Save(const std::string& s) {
#define SVS(nm, vl) n->addchild(#nm, vl)
#define SV(nm, vl) SVS(nm, std::to_string(vl))
	XmlNode head = {};
	head.name = "AViS_Graph_File";
	for (auto nd : nodes) {
		auto n = head.addchild("node");
		SV(type, (int)nd->script->type);
		SVS(name, nd->script->name);
		SV(tile, nd->canTile);
		nd->SaveConn();
		auto nc = n->addchild("conns");
		for (size_t a = 0; a < nd->_connInfo.size(); a++) {
			auto& c = nd->_connInfo[a];
			auto n = nc->addchild("item");
			SVS(name, c.mynm);
			SVS(type, c.mytp);
			SV(connd, c.cond);
			if (c.cond) {
				SV(tarid, c.tar->id);
				SVS(tarname, c.tarnm);
				SVS(tartype, c.tartp);
			}
			else {
				auto& tp = nd->script->invars[a].second;
				if (tp == "int") SV(value, nd->inputVDef[a].i);
				else if (tp == "double") SV(value, nd->inputVDef[a].d);
			}
		}
	}
	Xml::Write(&head, s);
#undef SVS
#undef SV
}

void AnWeb::SaveIn() {
	std::string path = IO::path + "nodes/__tmp__/";
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
	std::string path = IO::path + "ser/";
#else
	std::string path = IO::path + "nodes/__tmp__/";
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

void AnWeb::Load(const std::string& s) {
	XmlNode* head = Xml::Parse(s);
	if (!head) {
		Debug::Warning("AnWeb", "Cannot open save file!");
		return;
	}
	nodes.clear();
	AnNode* n = nullptr;
	AN_SCRTYPE tp;
	std::string nm;
	int cnt = 0;
	for (auto& nd : head->children[0].children) {
		if (nd.name != "node") continue;
		for (auto& c : nd.children) {
			if (c.name == "type") tp = (AN_SCRTYPE)TryParse(c.value, 0);
			else if (c.name == "name") {
				nm = c.value;
				switch (tp) {
				case AN_SCRTYPE::NONE:
		#define ND(scr) if (nm == scr::sig) n = new scr(); else
					ND(Node_Inputs)
					ND(Node_Info)
					ND(Node_AddBond)
					//ND(Node_AddVolume)
					ND(Node_TraceTrj)
					ND(Node_Camera_Out)
					ND(Node_SetParam)
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
					continue;
				}
				n->id = cnt++;
				nodes.push_back(n);
			}
			else if (c.name == "tile") n->canTile = (c.value == "1");
			
#define GTS(nm, vl) if (cc.name == #nm) vl = cc.value
#define GT(nm, vl) if (cc.name == #nm) vl = TryParse(cc.value, vl)
			else if (c.name == "conns") {
				for (auto& c1 : c.children) {
					if (c1.name != "item") continue;
					AnNode::ConnInfo ci;
					for (auto& cc : c1.children) {
						GTS(name, ci.mynm);
						else GTS(type, ci.mytp);
						else if (cc.name == "connd") ci.cond = (cc.value == "1");
						else if (ci.cond) {
							if (cc.name == "tarid") {
								auto i = TryParse(cc.value, 0xffff);
								if (i < cnt - 1) ci.tar = nodes[i];
							}
							else GTS(tarname, ci.tarnm);
							else GTS(tartype, ci.tartp);
						}
						else {
							if (cc.name == "value") {
								auto k = n->_connInfo.size();
								auto& vd = n->inputVDef[k];
								if (n->script->invars[k].second == "int") vd.i = TryParse(cc.value, -1);
								else vd.d = TryParse(cc.value, -1.0);
							}
						}
					}
					n->_connInfo.push_back(ci);
				}
			}
#undef GTS
#undef GT
		}
		n->Reconn();
		activeFile = s;
	}
}

void AnWeb::LoadIn() {
#ifdef IS_ANSERVER
	std::string path = IO::path + "ser/in/";
#else
	std::string path = IO::path + "nodes/__tmp__/in/";
#endif
	for (auto n : nodes) {
		n->LoadIn(path);
	}
}

void AnWeb::LoadOut() {
	std::string path = IO::path + "nodes/__tmp__/out/";
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

void AnWeb::Serialize(XmlNode* n) {
	n->name = "Analysis";
	n->addchild("graph", "graph.anl");
	Save(VisSystem::currentSavePath + "_data/graph.anl");
#define SVS(nm, vl) n->addchild(#nm, vl)
#define SV(nm, vl) SVS(nm, std::to_string(vl))
	SV(focus, drawFull);
#undef SVS
#undef SV
}

void AnWeb::Deserialize(XmlNode* nd) {
#define GT(nm, vl) if (n.name == #nm) vl = TryParse(n.value, vl)
#define GTV(nm, vl) if (n.name == #nm) Xml::ToVec(&n, vl)
	for (auto& n2 : nd->children) {
		if (n2.name == "Analysis") {
			for (auto& n : n2.children) {
				if (n.name == "graph") {
					Load(VisSystem::currentSavePath2 + n.value);
				}
				else if (n.name == "focus") drawFull = (n.value == "1");
			}
			return;
		}
	}
#undef GT
#undef GTV
}

void AnWeb::OnSceneUpdate() {
	for (auto n : nodes) {
		n->OnSceneUpdate();
	}
}

void AnWeb::OnAnimFrame() {
	ReadFrame(Particles::anim.currentFrame);
	for (auto n : nodes) {
		n->OnAnimFrame();
	}
}