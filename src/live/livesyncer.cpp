#include "livesyncer.h"
#include "md/particles.h"
#include "vis/pargraphics.h"
#include "lj256.h"
#include "imp/parloader.h"
#include "ui/icons.h"

LiveSyncer::LIVE_STATUS LiveSyncer::status = LiveSyncer::MENU;
std::vector<LiveRunner*> LiveSyncer::runners;
LiveRunner* LiveSyncer::activeRunner;
SyncInfo LiveSyncer::info = {};

uint LiveSyncer::tarFrm;
bool LiveSyncer::applyFrm;

bool LiveSyncer::expanded = true;
float LiveSyncer::expandPos;

std::thread* LiveSyncer::runThread;

void LiveSyncer::Init(uint _i) {
	auto& r = runners[_i];
	if (!r->lib) {
		r->lib = new DyLib(r->path);
		if (!r->lib) {
			runners.erase(runners.begin() + _i);
			return;
		}
		r->initFunc = (LiveRunner::initSig)r->lib->GetSym(r->initNm);
		r->loopFunc = (LiveRunner::loopSig)r->lib->GetSym(r->loopNm);
		if (!r->initFunc || !r->loopFunc) {
			runners.erase(runners.begin() + _i);
			return;
		}
	}

	info = {};
	info.namesz = PAR_MAX_NAME_LEN;
	if (!r->initFunc(&info)) {
		return;
	}
	activeRunner = r;
	
	//fix this part

	ParLoader::parDirty = true;
	status = IDLE;
}

void LiveSyncer::Start() {
	if (!activeRunner) return;
	status = LOOP;
	if (!runThread) runThread = new std::thread(DoRun);
}

void LiveSyncer::Update() {
	if (status == FAIL) {
		Stop();
	}
	else if (applyFrm) {
		Particles::SetFrame(tarFrm);
		applyFrm = false;
	}
}

void LiveSyncer::Pause() {
	status = PAUSE;
}

void LiveSyncer::Stop() {
	status = IDLE;
	if (runThread->joinable()) runThread->join();
	delete(runThread);
	runThread = nullptr;
	applyFrm = false;
	Particles::SetFrame(0);
}

void LiveSyncer::DrawSide() {
	UI::Quad(Display::width - expandPos, 18, 180, Display::height - 36.f, white(0.9f, 0.15f));
	if (expanded) {
		UI::Label(Display::width - expandPos + 5, 20, 12, "Live Run", white());

		float nma = (status == MENU) ? 0.5f : 1;

		if (Engine::Button(Display::width - expandPos + 5, 38, expandPos - 10, 16, white(nma, 0.6f)) == MOUSE_RELEASE) {
			if (status != MENU) {
				if (status == LOOP) {
					Pause();
				}
				else {
					Start();
				}
			}
		}
		if (status == LOOP) UI::Texture(Display::width - expandPos * 0.5f - 8, 38, 16, 16, Icons::pause, yellow());
		else UI::Texture(Display::width - expandPos * 0.5f - 8, 38, 16, 16, Icons::play);
		if (Engine::Button(Display::width - expandPos + 5, 17 * 3 + 4, expandPos - 10, 16, Vec4(0.7f, 0.4f, 0.4f, nma)) == MOUSE_RELEASE) {
			if (status > IDLE) Stop();
		}
		UI::Quad(Display::width - expandPos * 0.5f - 5, 17 * 3 + 7, 10, 10, red());
		if (status == MENU) {
			UI::Label(Display::width - expandPos + 2, 17 * 5 + 4, 12, "Select Module", white());
			uint a = 0;
			for (auto& r : runners) {
				if (Engine::Button(Display::width - expandPos + 4, 17 * 6 + 5.f + 17 * a, 170, 16, white(1, 0.6f), r->name, 12, white()) == MOUSE_RELEASE) {
					Init(a);
				}
				a++;
			}
		}

		if ((!UI::editingText && Input::KeyUp(KEY::R)) || Engine::Button(Display::width - expandPos - 16.f, Display::height - 34.f, 16.f, 16.f, Icons::collapse, white(0.8f), white(), white(0.5f)) == MOUSE_RELEASE)
			expanded = false;
		expandPos = Clamp(expandPos + 1500 * Time::delta, 0.f, 180.f);
	}
	else {
		UI::Quad(Display::width - expandPos, 0.f, expandPos, Display::height - 18.f, white(0.9f, 0.15f));
		if ((!UI::editingText && Input::KeyUp(KEY::R)) || Engine::Button(Display::width - expandPos - 110.f, Display::height - 34.f, 110.f, 16.f, white(0.9f, 0.15f), white(1, 0.15f), white(1, 0.05f)) == MOUSE_RELEASE)
			expanded = true;
		UI::Texture(Display::width - expandPos - 110.f, Display::height - 34.f, 16.f, 16.f, Icons::expand);
		UI::Label(Display::width - expandPos - 92.f, Display::height - 33.f, 12.f, "Analysis (A)", white());
		expandPos = Clamp(expandPos - 1500 * Time::delta, 2.f, 180.f);
	}
}

void LiveSyncer::DoRun() {
	auto& anm = Particles::anim;
	while (status > IDLE) {
		if (status == LOOP) {
			if (anm.currentFrame == 1) {
				//info.pos = (float*)anm.poss[2];
				//info.vel = (float*)anm.vels[2];
			}
			else {
				//info.pos = (float*)anm.poss[1];
				//info.vel = (float*)anm.vels[1];
			}
			info.fill = false;
			if (!activeRunner->loopFunc(&info))
				status = FAIL;
			else if (info.fill) {
				tarFrm = (anm.currentFrame == 1) ? 2 : 1;
				applyFrm = true;
				while (applyFrm && (status > IDLE));
			}
		}
		else {
			Engine::Sleep(100);
		}
	}
}