// Copyright (C) 2019 Pua Kai
// 
// This file is part of AViS.
// 
// AViS is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// AViS is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with AViS.  If not, see <http://www.gnu.org/licenses/>.

#include "anweb.h"
#include "nodes/get/node_inputs.h"
#include "nodes/get/node_info.h"
#ifndef IS_ANSERVER
#include "anops.h"
#include "errorview.h"
#include "ui/localizer.h"
#include "ui/icons.h"
#include "ui/ui_ext.h"
#include "md/particles.h"
#include "imp/parloader.h"
#include "vis/pargraphics.h"
#include "vis/system.h"
#include "utils/dialog.h"
#include "utils/tinyfiledialogs.h"
#endif

#define NO_REDIR_LOG
#define VERBOSE

bool AnWeb::lazyLoad = true;

std::string AnWeb::nodesPath = "";

AnNode* AnWeb::selConnNode = nullptr;
uint AnWeb::selConnId = 0;
bool AnWeb::selConnIdIsOut = false, AnWeb::selPreClear = false;
Vec2 AnWeb::selConnPos;
Vec4 AnWeb::selConnCol;
AnScript* AnWeb::selScript = nullptr;
uint AnWeb::selSpNode = 0;

std::string AnWeb::activeFile = "";
std::vector<pAnNode> AnWeb::nodes;

std::mutex AnWeb::execNLock;
int AnWeb::execN;
bool AnWeb::abortExec;

bool AnWeb::drawFull = false, AnWeb::expanded = true;
bool AnWeb::executing = false;
bool AnWeb::apply = false;
float AnWeb::maxScroll, AnWeb::scrollPos = 0, AnWeb::expandPos = 0;
int AnWeb::execFrame, AnWeb::realExecFrame, AnWeb::execdFrame;
float AnWeb::drawLerp;
bool AnWeb::invertRun = false, AnWeb::runOnFrame = false;
bool AnWeb::highContrast = false;

float AnWeb::zoomOut = 0;
float AnWeb::zoomRatio;

std::thread* AnWeb::execThread = nullptr;
AnNode* AnWeb::execNode = nullptr;
uint AnWeb::currNode = 0, AnWeb::nextNode = 0;

bool AnWeb::hasPy = false, AnWeb::hasC = false, AnWeb::hasFt = false;
bool AnWeb::hasPy_s = false, AnWeb::hasC_s = false, AnWeb::hasFt_s = false;

bool AnWeb::waitBrowse = false;

void AnWeb::Init() {
	nodesPath = VisSystem::localFd + "nodes/";
	if (!IO::HasDirectory(nodesPath))
		IO::MakeDirectory(nodesPath);
	Clear0();
	for (int a = 0; a < 10; ++a) {
		AnBrowse::mscFdExpanded[a] = true;
	}
	ChokoLait::focusFuncs.push_back(CheckChanges);
}

void AnWeb::Clear() {
	nodes.clear();
}

void AnWeb::Clear0() {
	Clear();
	nodes.push_back(Node_Inputs::_Spawn());
	nodes.push_back(Node_Info::_Spawn());
	nodes[0]->canTile = nodes[1]->canTile = true;
}

void AnWeb::Insert(const pAnNode& node, Vec2 pos) {
	nodes.push_back(node);
	node->pos = pos;
}

void AnWeb::Update() {
#ifndef IS_ANSERVER
	if (waitBrowse) {
		if (!AnBrowse::busy && !AnBrowse::changed) {
			waitBrowse = false;
			ClearConn();
			Reconn();
		}
	}
	else {
		for (auto n : nodes) {
			n->Update();
		}
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
	}
#endif
	AnBrowse::Update();
}

void AnWeb::Draw() {
#ifndef IS_ANSERVER
	const float alpha = VisSystem::opacity;
	const float maxwidth = Display::width - AnBrowse::expandPos - AnOps::expandPos;
	UI::Quad(AnBrowse::expandPos, 0.f, maxwidth, Display::height - 18.f, highContrast ? white() : white(alpha * drawLerp, 0.05f));
	static float scrollFade = 3;
	if (!waitBrowse) {
		Engine::BeginStencil(AnBrowse::expandPos, 0.f, Lerp(Display::width - AnBrowse::expandPos - AnOps::expandPos, maxScroll, zoomOut), Display::height - 18.f);
		byte ms = Input::mouse0State;
		AnNode::width = 220;
		if (executing) {
			Input::mouse0 = false;
			Input::mouse0State = 0;
		}
		AnWeb::selPreClear = true;
		
		Vec2 poss(AnBrowse::expandPos + 10 - scrollPos * (1 - zoomOut), 100);

		if (zoomOut > 0) {
			const float r = Lerp(1.f, zoomRatio, zoomOut);
			UI::matrix = glm::mat3(1, 0, 0, 0, 1, 0, poss.x, poss.y, 1)
				* glm::mat3(r, 0, 0, 0, r, 0, 0, 0, 1)
				* glm::mat3(1, 0, 0, 0, 1, 0, -poss.x, -poss.y, 1);
			UI::matrixIsI = false;

			UI::Quad(poss.x - 10 + scrollPos, poss.y - 50, maxwidth, Display::height - poss.y, white(0.1f * zoomOut * std::min(scrollFade, 1.f)));
			if (!Input::mouseScroll) scrollFade = std::max(scrollFade - 2 * Time::delta, 0.f);
			else scrollFade = 3;
		}

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
			n->DrawBack();
			offy = n->height;
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
			n->DrawConn();
		}
		for (auto n : nodes) {
			n->Draw();
		}
		AnNode::DrawMouseConn();
		if (zoomOut > 0) UI::ResetMatrix();
		if (Input::mouse0State == MOUSE_UP && selPreClear) selConnNode = nullptr;

		if (Input::mousePos.x > AnBrowse::expandPos && Input::mousePos.x < Display::width - AnOps::expandPos) {
			float canScroll = std::max(maxScroll - (Display::width - AnBrowse::expandPos - AnOps::expandPos), 0.f);
			scrollPos = Clamp(scrollPos - Input::mouseScroll * 1000 * Time::delta, 0.f, canScroll);
		}

		Input::mouse0State = ms;
		Input::mouse0 = (Input::mouse0State == 1) || (Input::mouse0State == 2);
		Engine::EndStencil();

		if (Input::mouse0State == MOUSE_UP) {
			if (selScript) {
				if (iter >= 0) {
					pAnNode pn;
					if ((uintptr_t)selScript > 1) {
						switch (selScript->type) {
						case AnScript::TYPE::C:
							pn = std::make_shared<CNode>(std::dynamic_pointer_cast<CScript_I>(selScript->CreateInstance()));
							break;
						case AnScript::TYPE::PYTHON:
							pn = std::make_shared<PyNode>(std::dynamic_pointer_cast<PyScript_I>(selScript->CreateInstance()));
							break;
						case AnScript::TYPE::FORTRAN:
							pn = std::make_shared<FNode>(std::dynamic_pointer_cast<FScript_I>(selScript->CreateInstance()));
							break;
						default:
							Debug::Error("AnWeb::Draw", "Unhandled script type: " + std::to_string((int)selScript->type));
							break;
						}
					}
					else {
						pn = AnNode_Internal::scrs[selSpNode >> 8][selSpNode & 255].spawner();
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
						nodes.erase(nn);
						break;
					}
				}
			}
		}

		if (selScript) {
			Texture icon = Icons::lang_ft;
			if ((uintptr_t)selScript == 1)
				icon = Icons::lightning;
			else if (selScript->type == AnScript::TYPE::C)
				icon = Icons::lang_c;
			else if (selScript->type == AnScript::TYPE::PYTHON)
				icon = Icons::lang_py;
			UI::Texture(Input::mousePos.x - 16, Input::mousePos.y - 16, 32, 32, icon, white(0.3f));
		}

		float wo = 200;
		bool haf = activeFile != "";
		if (!executing && Engine::Button(wo, 1, 70, 16, white(1, haf ? 0.4f : 0.2f), _("Save"), 12.f, white(), true) == MOUSE_RELEASE)
			if (haf) Save(activeFile);
		wo += 75;
		if (!executing && Engine::Button(wo, 1, 70, 16, white(1, 0.4f), _("Save As"), 12.f, white(), true) == MOUSE_RELEASE)
			Save(Dialog::SaveFile(EXT_ANSV));
		wo += 75;
		if (!executing && Engine::Button(wo, 1, 70, 16, white(1, 0.4f), _("Open"), 12.f, white(), true) == MOUSE_RELEASE) {
			auto res = Dialog::OpenFile({ "*" EXT_ANSV });
			if (!!res.size()) Load(res[0]);
		}
		wo += 75;
		if (!executing && Engine::Button(wo, 1, 70, 16, white(1, 0.4f), _("Clear"), 12, white(), true) == MOUSE_RELEASE) {
			Clear0();
		}
		wo += 75;
		bool canexec = !Particles::empty && (!AnOps::remote || (AnOps::connectStatus == 255)) && !executing && !ParLoader::busy && !AnBrowse::busy;
		if (Engine::Button(wo, 1, 70, 16, white(1, canexec ? 0.4f : 0.2f), _("Run"), 12, white(), true) == MOUSE_RELEASE) {
			if (canexec) AnWeb::Execute(false);
		}
		UI::Texture(wo, 1, 16, 16, Icons::play);
		wo += 75;
		bool canexec2 = (canexec && Particles::anim.frameCount > 1);
		if (Engine::Button(wo, 1, 107, 16, white(1, canexec2 ? 0.4f : 0.2f), _("Run All"), 12, white(), true) == MOUSE_RELEASE) {
			if (canexec2) {
				AnWeb::Execute(true);
				//tinyfd_messageBox("Warning", "Please do not press that button again.", "ok", "warning", 1);
			}
		}
		UI::Texture(wo, 1, 16, 16, Icons::playall);
		if (drawLerp < 1) {
			drawLerp = std::min(drawLerp + 10 * Time::delta, 1.f);
			ParGraphics::tfboDirty = true;
		}
	}

	zoomRatio = (Display::width - 350.f) / maxScroll;
	if (zoomRatio < 1) {
		if (!zoomOut) {
			if (Engine::Button(Display::width - AnOps::expandPos - 30, 5, 16, 16, Icons::zoomOut) == MOUSE_RELEASE) {
				zoomOut = 0.001f;
				scrollFade = 3;
			}
		}
		else {
			zoomOut = Lerp(zoomOut, 1.f, 20 * Time::delta);
			if (Engine::Button(Display::width - AnOps::expandPos - 30, 5, 16, 16, Icons::zoomIn) == MOUSE_RELEASE
					|| selScript) {
				zoomOut = 0;
			}
		}
	}

	AnBrowse::Draw();
	AnOps::Draw();

	if (waitBrowse) {
		UI::IncLayer();
		UI::Quad(0, 0, (float)Display::width, (float)Display::height, black(0.5f));
		UI::font.Align(ALIGN_MIDCENTER);
		UI::Label(Display::width * 0.5f, Display::height * 0.5f, 12, AnBrowse::busyMsg, white(0.8f));
		UI::font.Align(ALIGN_TOPLEFT);
	}

	if (Input::KeyDown(KEY::Escape) || (Engine::Button(Display::width - 71.f, 1.f, 70.f, 16.f, white(1, 0.4f), _("Done"), 12.f, white(), true) == MOUSE_RELEASE)) {
		if (selScript) selScript = nullptr;
		else {
			drawFull = false;
			AnBrowse::expandPos = AnOps::expandPos = 0;
			ParGraphics::tfboDirty = true;
		}
	}
#endif
}

void AnWeb::DrawSide() {
#ifndef IS_ANSERVER
	const float alpha = VisSystem::opacity;
	const float expos = Display::width - expandPos;
	UI2::BackQuad(expos, 18, 180.f, Display::height - 36.f);
	if (expanded) {
		float w = 180;
		AnNode::width = w - 2;
		UI::Label(expos + 5, 20, 12, _("Analysis"), white());

		if (Engine::Button(expos + 109, 20, 70, 16, white(1, 0.4f), _("Edit"), 12, white(), true) == MOUSE_RELEASE){
			drawFull = true;
		}

		if (!waitBrowse) {
			bool canexec = !Particles::empty && (!AnOps::remote || (AnOps::connectStatus == 255)) && !executing && !ParLoader::busy && !AnBrowse::busy;
			if (Engine::Button(expos + 1, 38, 70, 16, white(1, canexec ? 0.4f : 0.2f), _("Run"), 12, white(), true) == MOUSE_RELEASE) {
				if (canexec) AnWeb::Execute(false);
			}
			UI::Texture(expos + 1, 38, 16, 16, Icons::play);
			bool canexec2 = (canexec && Particles::anim.frameCount > 1);
			if (!execFrame) {
				if (Engine::Button(expos + 72, 38, 107, 16, white(1, canexec2 ? 0.4f : 0.2f), _("Run All"), 12, white(), true) == MOUSE_RELEASE) {
					if (canexec2) {
						AnWeb::Execute(true);
						//tinyfd_messageBox("Warning", "Please do not press that button again.", "ok", "warning", 1);
					}
				}
			}
			else {
				UI::Quad(expos + 72, 38, 107, 16, white(1, 0.2f));
				UI::font.Align(ALIGN_TOPCENTER);
				UI::Label(expos + 72 + 54, 39, 12, std::to_string(execFrame), white(0.8f));
				UI::font.Align(ALIGN_TOPLEFT);
			}
			if (invertRun)
				UI::Texture(expos + 178, 38, -16, 16, Icons::playall);
			else
				UI::Texture(expos + 72, 38, 16, 16, Icons::playall);

			float off = UI::BeginScroll(expos, 17 * 3 + 4, 180, Display::height - 17.f * 3 - 23.f);
			Vec2 poss(expos + 1, off);
			for (auto n : nodes) {
				n->pos = poss;
				poss.y += n->DrawSide() + 1;
			}
			UI::EndScroll(poss.y);
			UI2::BackQuad(expos - 16.f, Display::height - 34.f, 16, 16);
			if ((!UI::editingText && Input::KeyUp(KEY::A)) || Engine::Button(expos - 16.f, Display::height - 34.f, 16.f, 16.f, Icons::collapse) == MOUSE_RELEASE)
				expanded = false;
			expandPos = Clamp(expandPos + 1500 * Time::delta, 0.f, 180.f);
		}
	}
	else {
		if ((!UI::editingText && Input::KeyUp(KEY::A)) || Engine::Button(expos - 110.f, Display::height - 34.f, 110, 16, white(alpha, 0.15f), white(alpha * 2, 0.15f), white(alpha / 2, 0.05f)) == MOUSE_RELEASE)
			expanded = true;
		UI::Texture(expos - 110.f, Display::height - 34.f, 16.f, 16.f, Icons::expand);
		UI::Label(expos - 92.f, Display::height - 33.f, 12.f, _("Analysis") +" (A)", white());
		expandPos = Clamp(expandPos - 1500 * Time::delta, 2.f, 180.f);
	}
	drawLerp = 0;
#endif
}

void AnWeb::DrawScene(const RENDER_PASS pass) {
#ifndef IS_ANSERVER
	if (!waitBrowse) {
		for (auto& n : nodes) n->DrawScene(pass);
	}
#endif
}

void AnWeb::DrawOverlay() {
#ifndef IS_ANSERVER
	if (!waitBrowse) {
		for (auto& n : nodes) n->DrawOverlay();
	}
#endif
}

void AnWeb::Execute(bool all) {
	if (Particles::empty || executing) return;
	ParGraphics::animate = false;
	if (execThread) {
		if (execThread->joinable()) execThread->join();
		delete(execThread);
	}
	executing = true;
	abortExec = false;
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

	uint i = 0;
	for (auto n : nodes) {
		n->log.clear();
		n->id = i++;
	}
	ErrorView::execMsgs.clear();
	
	RemoveFrames();
	if (all) {
		auto f = Particles::anim.retainFrame = Particles::anim.currentFrame;
		ApplyFrameCount(Particles::anim.frameCount);
		for (uint _a = 0; _a < Particles::anim.frameCount; ++_a) {
			uint a = invertRun? Particles::anim.frameCount - _a - 1U : _a;
#ifdef VERBOSE
			Debug::Message("AnWeb", "Frame " + std::to_string(a));
#endif
			execFrame = a+1;
			realExecFrame = a;
			Particles::anim.Seek(a);
			auto st = Particles::anim.status[a];
			if (st != Particles::animdata::FRAME_STATUS::LOADED){
				Debug::Warning("AnWeb::Execute", "failed to seek to frame " + std::to_string(a) + "! " + std::to_string((int)st));
				executing = false;
				Engine::stateLock2.unlock();
				Particles::anim.Seek(f);
				execFrame = 0;
				return;
			}
			Particles::UpdateAttrs();
			_DoExecute();
			AnWeb::WriteFrame(a);
		}
		execFrame = 0;
		Particles::anim.retainFrame = ~0U;
		Particles::anim.Seek(f);
	}
	else {
		realExecFrame = Particles::anim.currentFrame;
		_DoExecute();
		execdFrame = (int)Particles::anim.currentFrame;
	}

	executing = false;
	apply = true;

	Engine::stateLock2.unlock();
}

void AnWeb::_DoExecute() {
	char* err = 0;
	static std::string pylog;
	nextNode = ~0U;
	execN = 0;
	for (auto& n : nodes) {
		n->PreExecute();
	}
	for (auto& n : nodes) {
		n->TryExecute();
	}
	while (execN > 0) {
		Engine::Sleep(100);
	}
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
#define SVS(nm, vl) n->addchild(#nm, vl)
#define SV(nm, vl) SVS(nm, std::to_string(vl))
void AnWeb::Save(const std::string& s) {
	XmlNode head = {};
	head.name = "AViS_Graph_File";
	for (auto nd : nodes) {
		auto n = head.addchild("node");
		SV(type, (int)nd->script->parent->type);
		SVS(name, nd->script->parent->name);
		SV(tile, nd->canTile);
		nd->Save(n->addchild("detail"));
		nd->SaveConn();
		auto nc = n->addchild("conns");
		for (size_t a = 0; a < nd->_connInfo.size(); ++a) {
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
				//auto& tp = nd->script->invars[a].second;
				//if (tp == "int") SV(value, nd->inputVDef[a].i);
				//else if (tp == "double") SV(value, nd->inputVDef[a].d);
				auto& vd = nd->script->defVals[a];
				if (nd->script->parent->inputs[a].type == AN_VARTYPE::INT)
					SV(value, vd.i);
				else
					SV(value, vd.d);
			}
		}
	}
	Xml::Write(&head, s);
	activeFile = s;
}
#undef SVS
#undef SV

void AnWeb::SaveIn() {
	std::string path = AnWeb::nodesPath + "__tmp__/";
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
	std::string path = AnWeb::nodesPath + "__tmp__/";
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
	pAnNode n;
	AnScript::TYPE tp;
	std::string nm;
	int cnt = 0;
	for (auto& nd : head->children[0].children) {
		if (nd.name != "node") continue;
		for (auto& c : nd.children) {
			if (c.name == "type") tp = (AnScript::TYPE)TryParse(c.value, 0);
			else if (c.name == "name") {
				nm = c.value;
				switch (tp) {
				case AnScript::TYPE::NONE:
					for (auto& ss : AnNode_Internal::scrs) {
						for (auto& a : ss) {
							if (nm == a.sig) {
								n = a.spawner();
								goto ifound;
							}
						}
					}
					Debug::Error("AnWeb::Load", "Unknown internal node: " + nm);
					return;
					ifound:
					break;
#define SCRCASE(tp, pre)\
				case AnScript::TYPE::tp:\
					n = std::make_shared<pre##Node>(std::dynamic_pointer_cast<pre##Script_I>\
						(pre##Script::allScrs[nm].lock()->CreateInstance()));\
					break;

					SCRCASE(PYTHON, Py);
					SCRCASE(C, C);
					SCRCASE(FORTRAN, F);

#undef SCRCASE
				default:
					Debug::Warning("AnWeb::Load", "Unknown node type: " + std::to_string((byte)tp));
					return;
				}
				if (!n->script) {
					Debug::Warning("AnWeb::Load", "Cannot find script \"" + nm + "\"!");
					return;
				}
				n->id = cnt++;
				nodes.push_back(n);
			}
			else if (c.name == "tile") n->canTile = (c.value == "1");
			else if (c.name == "detail") {
				n->Load(&c);
			}
			
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
								if (i < cnt - 1) ci.tar = nodes[i].get();
							}
							else GTS(tarname, ci.tarnm);
							else GTS(tartype, ci.tartp);
						}
						else {
							if (cc.name == "value") {
								auto k = n->_connInfo.size();
								auto& vd = n->script->defVals[k];
								if (n->script->parent->inputs[k].type == AN_VARTYPE::INT) vd.i = TryParse(cc.value, 0);
								else vd.d = TryParse(cc.value, 0.0);
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
	}
	activeFile = s;
}

void AnWeb::LoadIn() {
#ifdef IS_ANSERVER
	std::string path = IO::path + "ser/in/";
#else
	std::string path = AnWeb::nodesPath + "__tmp__/in/";
#endif
	for (auto n : nodes) {
		n->LoadIn(path);
	}
}

void AnWeb::LoadOut() {
	std::string path = AnWeb::nodesPath + "__tmp__/out/";
	for (auto n : nodes) {
		n->LoadOut(path);
	}
}

void AnWeb::CheckChanges() {
	SaveConn();
	waitBrowse = true;
	AnBrowse::Refresh();
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
	if (runOnFrame && (execdFrame != (int)Particles::anim.currentFrame)) {
		DoExecute(false);
		Scene::dirty = true;
	}
	else {
		ReadFrame(Particles::anim.currentFrame);
		for (auto n : nodes) {
			n->OnAnimFrame();
		}
	}
}

std::string AnWeb::ConvertName(const std::string& name) {
	if (!name.size()) return "";
	char c = name[0];
	std::string res(1, c);
	res.reserve(name.size());
	if (c >= 'a' && c <= 'z')
		res[0] = c - 'a' + 'A';
	int i = 1;
	while (!!(c = name[i++])) {
		if (c >= 'A' && c <= 'Z') {
			res.push_back(' ');
		}
		res.push_back(c);
	}
	return res;
}