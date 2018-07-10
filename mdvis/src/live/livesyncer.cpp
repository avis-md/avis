#include "livesyncer.h"
#include "md/Particles.h"
#include "vis/pargraphics.h"
#include "utils/rawvector.h"
#include "lj256.h"
#include "md/parloader.h"
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
	
	Particles::connSz = 0;
	Particles::particles_ResName = info.resname;
	Particles::particles_Name = info.name;
	memcpy(Particles::boundingBox, info.bounds, 6 * sizeof(float));
	Particles::particles_Col = new byte[info.num];
	Particles::particles_Rad = new float[info.num];
	Particles::particles_Res = new Int2[info.num];
	ParGraphics::rotCenter = Vec3(info.bounds[0] + info.bounds[1],
		info.bounds[2] + info.bounds[3],
		info.bounds[4] + info.bounds[5]) * 0.5f;

	auto rs = rawvector<ResidueList, uint>(Particles::residueLists, Particles::residueListSz);
	auto rsv = rawvector<Residue, uint>(Particles::residueLists->residues, Particles::residueLists->residueSz);
	//auto cn = rawvector<Int2, uint>(Particles::particles_Conn, Particles::connSz);

	uint64_t currResNm = -1;
	uint16_t currResId = -1;
	ResidueList* trs = 0;
	Residue* tr = 0;
	Vec3 vec;

	uint lastOff = 0, lastCnt = 0;

	std::ifstream* strm = 0;

	for (uint i = 0; i < info.num; i++) {
		auto id1 = info.type[i];//info.name[i * PAR_MAX_NAME_LEN];
		auto& resId = info.resId[i];
		uint64_t resNm = *((uint64_t*)(&info.resname[i * PAR_MAX_NAME_LEN])) & 0x000000ffffffffff;

		if (currResNm != resNm) {
			rs.push(ResidueList());
			trs = &Particles::residueLists[Particles::residueListSz - 1];
			rsv = rawvector<Residue, uint>(trs->residues, trs->residueSz);
			trs->name = string(info.resname + i * PAR_MAX_NAME_LEN, 5);
			currResNm = resNm;
		}

		if (currResId != resId) {
			if (!!i) {
				if (tr->type != 255) {
					lastOff = tr->offset;
					lastCnt = tr->cnt;
				}
				else {
					lastOff = i;
					lastCnt = 0;
				}
			}
			rsv.push(Residue());
			tr = &trs->residues[trs->residueSz - 1];
			tr->offset = i;
			tr->name = std::to_string(resId);
			tr->type = 255;//AminoAcidType((char*)&resNm);
			tr->cnt = 0;
			currResId = resId;
			tr->offset_b = Particles::connSz;
			tr->cnt_b = 0;
		}
		Vec3 vec = *((Vec3*)(&info.pos[i * 3]));
		int cnt = int(lastCnt + tr->cnt);
		/*
#pragma omp parallel for
		for (int j = 0; j < cnt; j++) {
			Vec3 dp = Particles::particles_Pos[lastOff + j] - vec;

			if (fabsf(dp.x) < 0.25f && fabsf(dp.y) < 0.25f && fabsf(dp.z) < 0.25f) {
				auto dst = glm::length2(dp);
				uint32_t id2 = info.type[lastOff + j];//Particles::particles_Name[(lastOff + j) * PAR_MAX_NAME_LEN];
				float bst = VisSystem::_bondLengths[id1 + (id2 << 16)];
				if (!bst) bst = VisSystem::_defBondLength;
				if (dst < bst) {
#pragma omp critical
					{
						cn.push(Int2(i, lastOff + j));
						tr->cnt_b++;
					}
				}
			}
		}
		*/
		for (byte b = 0; b < Particles::defColPalleteSz; b++) {
			if (id1 == Particles::defColPallete[b]) {
				Particles::particles_Col[i] = b;
				break;
			}
		}

		float rad = VisSystem::radii[id1][1];

		Particles::particles_Rad[i] = rad;
		Particles::particles_Res[i] = Int2(Particles::residueListSz - 1, trs->residueSz - 1);

		tr->cnt++;
	}
	Particles::particleSz = info.num;

	auto& anm = Particles::anim;
	anm.reading = true;

	anm.poss = new Vec3*[3];
	anm.poss[0] = new Vec3[3 * info.num];
	anm.vels = new Vec3*[3];
	anm.vels[0] = new Vec3[3 * info.num];
	memcpy(anm.poss[0], info.pos, info.num * sizeof(Vec3));
	memcpy(anm.vels[0], info.vel, info.num * sizeof(Vec3));
	Particles::particles_Pos = anm.poss[0];
	Particles::particles_Vel = anm.vels[0];
	anm.poss[1] = anm.poss[0] + info.num;
	anm.vels[1] = anm.vels[0] + info.num;
	anm.poss[2] = anm.poss[0] + info.num * 2;
	anm.vels[2] = anm.vels[0] + info.num * 2;

	anm.frameCount = 3;
	anm.reading = false;

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
	Engine::DrawQuad(Display::width - expandPos, 18, 180, Display::height - 36.0f, white(0.9f, 0.15f));
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
		Engine::DrawQuad(Display::width - expandPos * 0.5f - 5, 17 * 3 + 4, 10, 10, red());
		if (status == MENU) {
			UI::Label(Display::width - expandPos + 2, 17 * 5 + 4, 12, "Select Module", white());
			uint a = 0;
			for (auto& r : runners) {
				if (Engine::Button(Display::width - expandPos + 4, 17 * 4 + 5 + 17 * a, 170, 16, white(1, 0.6f), r->name, 12, white()) == MOUSE_RELEASE) {
					Init(a);
				}
				a++;
			}
		}

		if ((!UI::editingText && Input::KeyUp(Key_R)) || Engine::Button(Display::width - expandPos - 16.0f, Display::height - 34.0f, 16.0f, 16.0f, Icons::collapse, white(0.8f), white(), white(0.5f)) == MOUSE_RELEASE)
			expanded = false;
		expandPos = Clamp(expandPos + 1500 * Time::delta, 0.0f, 180.0f);
	}
	else {
		Engine::DrawQuad(Display::width - expandPos, 0.0f, expandPos, Display::height - 18.0f, white(0.9f, 0.15f));
		if ((!UI::editingText && Input::KeyUp(Key_R)) || Engine::Button(Display::width - expandPos - 110.0f, Display::height - 34.0f, 110.0f, 16.0f, white(0.9f, 0.15f), white(1, 0.15f), white(1, 0.05f)) == MOUSE_RELEASE)
			expanded = true;
		UI::Texture(Display::width - expandPos - 110.0f, Display::height - 34.0f, 16.0f, 16.0f, Icons::expand);
		UI::Label(Display::width - expandPos - 92.0f, Display::height - 33.0f, 12.0f, "Analysis (A)", white());
		expandPos = Clamp(expandPos - 1500 * Time::delta, 2.0f, 180.0f);
	}
}

void LiveSyncer::DoRun() {
	auto& anm = Particles::anim;
	while (status > IDLE) {
		if (status == LOOP) {
			if (anm.activeFrame == 1) {
				info.pos = (float*)anm.poss[2];
				info.vel = (float*)anm.vels[2];
			}
			else {
				info.pos = (float*)anm.poss[1];
				info.vel = (float*)anm.vels[1];
			}
			info.fill = false;
			if (!activeRunner->loopFunc(&info))
				status = FAIL;
			else if (info.fill) {
				tarFrm = (anm.activeFrame == 1) ? 2 : 1;
				applyFrm = true;
				while (applyFrm && (status > IDLE));
			}
		}
		else {
			Engine::Sleep(100);
		}
	}
}