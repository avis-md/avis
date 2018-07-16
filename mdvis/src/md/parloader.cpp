#include "parloader.h"
#include "Gromacs.h"
#include "Protein.h"
#include "vis/pargraphics.h"
#include "md/ParMenu.h"
#include "web/anweb.h"
#include "ui/icons.h"
#include "utils/rawvector.h"

#define EndsWith(s, s2) s.substr(sz - s2.size()) == s2

int ParLoader::impId, ParLoader::funcId;
ParImporter* ParLoader::customImp;
bool ParLoader::loadAsTrj = false, ParLoader::additive = false;
int ParLoader::maxframes = 0;
bool ParLoader::useConn, ParLoader::useConnCache, ParLoader::hasConnCache, ParLoader::oldConnCache, ParLoader::ovwConnCache;
string ParLoader::connCachePath;

std::vector<ParImporter*> ParLoader::importers;
std::vector<string> ParLoader::exts;

bool ParLoader::showDialog = false, ParLoader::busy = false, ParLoader::fault = false;
bool ParLoader::parDirty = false, ParLoader::trjDirty = false;
float* ParLoader::loadProgress = 0, *ParLoader::loadProgress2;
string ParLoader::loadName;
std::vector<string> ParLoader::droppedFiles;

bool ParLoader::_showImp = false;
float ParLoader::_impPos = 0, ParLoader::_impScr = 0;

void ParLoader::Init() {
	ChokoLait::dropFuncs.push_back(OnDropFile);
}

#if defined(PLATFORM_WIN)
#define OSFD "/win32/"
#define LIBEXT ".dll"
#elif defined(PLATFORM_LNX)
#define OSFD "/linux/"
#define LIBEXT ".so"
#elif defined(PLATFORM_OSX)
#define OSFD "/osx/"
#define LIBEXT ".so"
#endif

string _RMSP(string s) {
	string res;
	res.reserve(s.size());
	for (auto& c : s) {
		if (c != ' ') res += c;
	}
	return res;
}

void ParLoader::Scan() {
	string fd = IO::path + "/bin/importers/";
	std::vector<string> fds;
	IO::GetFolders(fd, &fds);
	exts.clear();
	for (auto& f : fds) {
		std::cout << "Import: " << f << std::endl;
		std::ofstream ostrm(fd + f + "/import.log");
		std::ifstream strm(fd + f + "/config.txt");
		if (strm.is_open()) {
			ParImporter* imp = new ParImporter();
			string s, libname;
			while (std::getline(strm, s)) {
				auto lc = s.find_first_of('=');
				if (lc != string::npos) {
					auto tp = s.substr(0, lc);
					auto vl = s.substr(lc + 1);
					if (tp == "name") imp->name = vl;
					else if (tp == "signature") imp->sig = vl;
					else if (tp == "file") {
						imp->lib = new DyLib(fd + f + OSFD + vl + LIBEXT);
						if (!imp->lib) {
							ostrm << "Importer lib file not found!";
							goto err;
						}
					}
				}
				else {
					if (!imp->lib) {
						ostrm << "Importer lib file must be defined before configurations!";
						goto err;
					}
					s = _RMSP(s);
					auto lc = s.find('{');
					if (lc != string::npos) {
						auto tp = s.substr(0, lc);
						if (tp == "configuration") {
							std::getline(strm, s);
							s = _RMSP(s);
							std::pair<std::vector<string>, ParImporter::loadsig> pr;
							int vlc = 0;
							while (s != "}") {
								auto lc = s.find_first_of('=');
								if (lc == string::npos) continue;
								auto tp = s.substr(0, lc);
								auto vl = s.substr(lc + 1);
								if (tp == "func") {
									if (!(pr.second = (ParImporter::loadsig)imp->lib->GetSym(vl))) {
										ostrm << "Importer function \"" << vl << "\" not found!";
										goto err;
									}
									vlc++;
								}
								else if (tp == "exts") {
									auto ob = vl.find('[');
									auto cb = vl.find(']');
									pr.first = string_split(vl.substr(ob + 1, cb - ob - 1), ',');
									vlc++;
								}
								std::getline(strm, s);
								s = _RMSP(s);
							}
							if (vlc == 2) {
								imp->funcs.push_back(pr);
							}
						}
						else if (tp == "trajectory") {
							std::getline(strm, s);
							s = _RMSP(s);
							std::pair<std::vector<string>, ParImporter::loadtrjsig> pr;
							int vlc = 0;
							while (s != "}") {
								auto lc = s.find_first_of('=');
								if (lc == string::npos) continue;
								auto tp = s.substr(0, lc);
								auto vl = s.substr(lc + 1);
								if (tp == "func") {
									if (!(pr.second = (ParImporter::loadtrjsig)imp->lib->GetSym(vl))) {
										ostrm << "Importer function \"" << vl << "\" not found!";
										goto err;
									}
									vlc++;
								}
								else if (tp == "exts") {
									auto ob = vl.find('[');
									auto cb = vl.find(']');
									pr.first = string_split(vl.substr(ob + 1, cb - ob - 1), ',');
									vlc++;
								}
								std::getline(strm, s);
								s = _RMSP(s);
							}
							if (vlc == 2) {
								imp->trjFuncs.push_back(pr);
							}
						}
					}
				}
			}
			if (!imp->name.size() || !imp->sig.size() || !imp->lib || !imp->funcs.size() || !imp->trjFuncs.size()) {
				ostrm << "Config contents incomplete!";
				goto err;
			}
			importers.push_back(imp);
			for (auto& a : imp->funcs) {
				exts.insert(exts.end(), a.first.begin(), a.first.end());
			}
			ostrm << "ok";
			continue;
		err:
			delete(imp);
		}
	}
}

void ParLoader::DoOpen() {
	busy = true;
	ParInfo info = {};
	info.path = info.trajectory.first = droppedFiles[0].c_str();
	info.nameSz = PAR_MAX_NAME_LEN;
	loadProgress = &info.progress;
	loadProgress2 = &info.trajectory.progress;
	loadName = "Reading file(s)";
	std::replace(droppedFiles[0].begin(), droppedFiles[0].end(), '\\', '/');
	string nm = droppedFiles[0].substr(droppedFiles[0].find_last_of('/') + 1);
	VisSystem::message = "Reading " + nm;

	auto t = milliseconds();

	//
	info.trajectory.maxFrames = 100;
	info.trajectory.frameSkip = 1;

	try {
		if (impId > -1) {
			if (!importers[impId]->funcs[funcId].second(&info)) {
				if (!info.error[0])
					Debug::Error("ParLoader", "Unspecified importer error!");
				else
					Debug::Error("ParLoader", "Importer error: " + string(info.error));
			}
		}
	}
	catch (char* c) {
		Debug::Error("ParLoader", "Importer exception: " + string(c));
		busy = false;
		fault = true;
	}

	*loadProgress2 = 0;

	Particles::connSz = 0;
	Particles::particles_ResName = info.resname;
	Particles::particles_Name = info.name;
	Particles::particles_Pos = (Vec3*)info.pos;
	Particles::particles_Vel = (Vec3*)info.vel;
	memcpy(Particles::boundingBox, info.bounds, 6 * sizeof(float));
	Particles::particles_Col = new byte[info.num];
	Particles::particles_Rad = new float[info.num];
	Particles::particles_Res = new Int2[info.num];
	ParGraphics::rotCenter = Vec3(info.bounds[0] + info.bounds[1], 
		info.bounds[2] + info.bounds[3], 
		info.bounds[4] + info.bounds[5]) * 0.5f;

	auto rs = rawvector<ResidueList, uint>(Particles::residueLists, Particles::residueListSz);
	auto rsv = rawvector<Residue, uint>(Particles::residueLists->residues, Particles::residueLists->residueSz);
	auto cn = rawvector<Int2, uint>(Particles::particles_Conn, Particles::connSz);

	uint64_t currResNm = -1;
	uint16_t currResId = -1;
	ResidueList* trs = 0;
	Residue* tr = 0;
	Vec3 vec;

	uint lastOff = 0, lastCnt = 0;

	std::ifstream* strm = 0;

	if (useConn) {
		if (useConnCache && hasConnCache && !ovwConnCache) {
			strm = new std::ifstream(droppedFiles[0] + ".conn", std::ios::binary);
			char buf[100];
			strm->getline(buf, 100, '\n');
			strm->read((char*)(&Particles::connSz), 4);
			Particles::particles_Conn = (Int2*)std::realloc(Particles::particles_Conn, Particles::connSz * sizeof(Int2));
			strm->read((char*)Particles::particles_Conn, Particles::connSz * sizeof(Int2));
			strm->read(buf, 2);
			if (buf[0] != 'X' || buf[1] != 'X') {
				useConnCache = false;
				loadName = "Finding bonds";
			}
			else loadName = "Reading bonds";
		}
		else loadName = "Finding bonds";
	}
	else loadName = "Post processing";
	for (uint i = 0; i < info.num; i++) {
		info.progress = i * 1.0f / info.num;
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
			tr->type = AminoAcidType((char*)&resNm);
			tr->cnt = 0;
			currResId = resId;
			if (useConnCache && hasConnCache && !ovwConnCache) {
				_Strm2Val(*strm, tr->offset_b);
				_Strm2Val(*strm, tr->cnt_b);
			}
			else {
				tr->offset_b = Particles::connSz;
				tr->cnt_b = 0;
			}
		}
		Vec3 vec = *((Vec3*)(&info.pos[i * 3]));
		int cnt = int(lastCnt + tr->cnt);
		if (useConn && (!useConnCache || !hasConnCache || ovwConnCache)) {
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
		}
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

	if (strm) delete(strm);

	if (useConnCache && (!hasConnCache || ovwConnCache)) {
		std::ofstream ostrm(droppedFiles[0] + ".conn", std::ios::binary);
		ostrm << __DATE__ << " " << __TIME__ << "\n";
		_StreamWrite(&Particles::connSz, &ostrm, 4);
		_StreamWrite(Particles::particles_Conn, &ostrm, Particles::connSz * sizeof(Int2));
		_StreamWrite("XX", &ostrm, 2);
		for (uint a = 0; a < Particles::residueListSz; a++) {
			auto& rsl = Particles::residueLists[a];
			for (uint b = 0; b < rsl.residueSz; b++) {
				_StreamWrite(&rsl.residues[b].offset_b, &ostrm, sizeof(uint));
				_StreamWrite(&rsl.residues[b].cnt_b, &ostrm, sizeof(ushort));
			}
		}
	}

	Particles::particleSz = info.num;

	if (info.trajectory.frames > 0) {
		auto& trj = info.trajectory;
		auto& anm = Particles::anim;
		anm.reading = true;

		anm.poss = new Vec3*[trj.frames];
		anm.poss[0] = new Vec3[trj.frames * info.num];
		anm.vels = new Vec3*[trj.frames];
		anm.vels[0] = new Vec3[trj.frames * info.num];
		for (uint16_t i = 0; i < trj.frames; i++) {
			anm.poss[i] = anm.poss[0] + info.num * i;
			memcpy(anm.poss[i], trj.poss[i], info.num * sizeof(Vec3));
			delete[](trj.poss[i]);
			anm.vels[i] = anm.vels[0] + info.num * i;
			if (trj.vels) {
				memcpy(anm.vels[i], trj.vels[i], info.num * sizeof(Vec3));
				delete[](trj.vels[i]);
			}
		}
		delete[](trj.poss);
		if (trj.vels) delete[](trj.vels);

		anm.frameCount = trj.frames;
		anm.reading = false;
	}

	VisSystem::message = "Loaded " + nm + " in " + std::to_string((milliseconds() - t)*0.001f).substr(0, 5) + "s";
	ParMenu::SaveRecents(droppedFiles[0]);
	ParLoader::parDirty = true;
	busy = false;
	fault = false;
}

void ParLoader::DoOpenAnim() {
	busy = true;
	auto& anm = Particles::anim;
	anm.reading = true;

	TrjInfo info = {};
	info.first = droppedFiles[0].c_str();
	info.parNum = Particles::particleSz;
	info.maxFrames = maxframes;

	if (impId > -1) importers[impId]->trjFuncs[funcId].second(&info);

	if (!info.frames) {
		busy = false;
		fault = true;
	}

	anm.poss = new Vec3*[info.frames];
	anm.poss[0] = new Vec3[info.frames * info.parNum];
	anm.vels = new Vec3*[info.frames];
	anm.vels[0] = new Vec3[info.frames * info.parNum];
	for (uint16_t i = 0; i < info.frames; i++) {
		anm.poss[i] = anm.poss[0] + info.parNum * i;
		memcpy(anm.poss[i], info.poss[i], info.parNum * sizeof(Vec3));
		delete[](info.poss[i]);
		anm.poss[i] = anm.poss[0] + info.parNum * i;
		if (info.vels) {
			memcpy(anm.poss[i], info.poss[i], info.parNum * sizeof(Vec3));
			delete[](info.poss[i]);
		}
	}
	delete[](info.poss);
	if (info.vels) delete[](info.vels);

	anm.frameCount = info.frames;

	anm.reading = false;
	busy = false;
	fault = false;
}

void ParLoader::DrawOpenDialog() {
	if (!showDialog) return;
	UI::IncLayer();
	Engine::DrawQuad(0, 0, (float)Display::width, (float)Display::height, black(0.7f));

	float woff = roundf(Display::width*0.5f - 200 - _impPos / 2);
	float hoff = roundf(Display::height * 0.5f - 150);

	if (_showImp || _impPos > 0) {
		Engine::DrawQuad(woff + 400, hoff, _impPos, 300, white(0.8f, 0.1f));
		Engine::PushStencil(woff + 400, hoff, _impPos, 300);
		UI::Label(woff + 402, hoff, 12, "Choose Importer", white());
		uint i = 1;
		for (auto& p : importers) {
			if (Engine::Button(woff + 401, hoff + 17 * i, 298, 16, (impId == i - 1) ? Vec4(0.5f, 0.4f, 0.2f, 1) : white(0.4f), p->name, 12, white()) == MOUSE_RELEASE) {
				impId = i - 1;
				funcId = 0;
				int id2 = 0;
				auto sz = droppedFiles[0].size();
				for (auto& pr : p->funcs) {
					for (auto& s : pr.first) {
						if (EndsWith(droppedFiles[0], s)) {
							funcId = id2;
							goto found;
						}
					}
					id2++;
				}
			found:;
			}
			i++;
		}
		Engine::PopStencil();
	}
	_impPos = _showImp ? min(_impPos + 800 * Time::delta, 100.0f) : max(_impPos - 800 * Time::delta, 0.0f);

	Engine::DrawQuad(woff, hoff, 400, 300, white(0.8f, 0.15f));
	Engine::DrawQuad(woff, hoff, 400, 16, white(0.9f, 0.1f));
	UI::Label(woff + 2, hoff, 12, loadAsTrj ? "Load Trajectory" : "Load Configuration", white());
	UI::Label(woff + 2, hoff + 17, 12, "File(s)", white());
	string nm = droppedFiles[0];
	if (Engine::Button(woff + 60, hoff + 17, 339, 16, white(1, 0.4f)) == MOUSE_RELEASE) {

	}
	UI::Label(woff + 62, hoff + 17, 12, nm, white(0.7f), 326);
	UI::Texture(woff + 383, hoff + 17, 16, 16, Icons::browse);

	UI::Label(woff + 2, hoff + 17 * 2, 12, "Importer", white(), 326);
	if (impId > -1)
		UI::Label(woff + 60, hoff + 34, 12, importers[impId]->name + " (" + importers[impId]->sig + ")", white(0.5f), 326);
	if (Engine::Button(woff + 339, hoff + 34, 60, 16, white(1, 0.4f), _showImp ? "<<" : ">>", 12, white(), true) == MOUSE_RELEASE) {
		_showImp = !_showImp;
	}
	UI::Label(woff + 2, hoff + 17 * 3, 12, "Module", white(), 326);
	if (impId > -1) {
		auto& ii = importers[impId];
		uint i = 0;
		if (loadAsTrj) {
			for (auto& f : ii->trjFuncs) {
				if (Engine::Button(woff + 60 + 50 * i, hoff + 17 * 3, 45, 16, (funcId == i) ? Vec4(0.5f, 0.4f, 0.2f, 1) : white(0.4f), f.first[0], 12, white(), true) == MOUSE_RELEASE) {
					funcId = i;
				}
				i++;
			}
		}
		else {
			for (auto& f : ii->funcs) {
				if (Engine::Button(woff + 60 + 50 * i, hoff + 17 * 3, 45, 16, (funcId == i)? Vec4(0.5f, 0.4f, 0.2f, 1) : white(0.4f), f.first[0], 12, white(), true) == MOUSE_RELEASE) {
					funcId = i;
				}
				i++;
			}
		}
	}
	//if (Particles::particleSz) {
		auto l = Engine::Toggle(woff + 5, hoff + 17 * 4, 16, Icons::checkbox, loadAsTrj, white(), ORIENT_HORIZONTAL);
		if (l != loadAsTrj) {
			loadAsTrj = l;
			FindImpId(true);
		}
		UI::Label(woff + 34, hoff + 17 * 4, 12, "As Trajectory", white(), 326);
		additive = Engine::Toggle(woff + 201, hoff + 17 * 4, 16, Icons::checkbox, additive, white(), ORIENT_HORIZONTAL);
		UI::Label(woff + 230, hoff + 17 * 4, 12, "Additive", white(), 326);
	//}
	UI::Label(woff + 2, hoff + 17 * 5, 12, "Options", white(), 326);
	useConn = Engine::Toggle(woff + 5, hoff + 17 * 6, 16, Icons::checkbox, useConn, white(), ORIENT_HORIZONTAL);
	UI::Label(woff + 25, hoff + 17 * 6, 12, "Bonds", white(), 326);
	if (useConn) {
		useConnCache = Engine::Toggle(woff + 100, hoff + 17 * 6, 16, Icons::checkbox, useConnCache, white(), ORIENT_HORIZONTAL);
		UI::Label(woff + 123, hoff + 17 * 6, 12, "Use Bond Cache", white(), 326);
		if (useConnCache) {
			UI::Label(woff + 5, hoff + 17 * 7, 12, hasConnCache ? "Cache found" + (string)(oldConnCache ? "(outdated): " : ": ") + connCachePath : "No cache found", white(), 326);
			if (hasConnCache) {
				ovwConnCache = Engine::Toggle(woff + 300, hoff + 17 * 7, 16, Icons::checkbox, ovwConnCache, white(), ORIENT_HORIZONTAL);
				UI::Label(woff + 320, hoff + 17 * 7, 12, "Overwrite", white(), 326);
			}
		}
	}

	string line = "";
	if (loadAsTrj) line += "-trj ";
	if (additive) line += "-a";
	if (maxframes > 0) line += "-n" + std::to_string(maxframes) + " ";
	UI::Label(woff + 2, hoff + 300 - 17 * 2, 12, "Command line : " + line, white(), 326);
	if (Engine::Button(woff + 300, hoff + 283, 48, 16, yellow(1, 0.4f), "Cancel", 12, white(), true) == MOUSE_RELEASE) {
		showDialog = false;
	}
	if (Engine::Button(woff + 350, hoff + 283, 49, 16, white(0.4f), "Load", 12, white(), true) == MOUSE_RELEASE) {
		if (loadAsTrj) {
			std::thread td(DoOpenAnim);
			td.detach();
		}
		else {
			Particles::Clear();
			std::thread td(DoOpen);
			td.detach();
		}
		showDialog = false;
	}
}

bool ParLoader::OnDropFile(int i, const char** c) {
	if (AnWeb::drawFull) return false;
	std::vector<string> fs(i);
	for (i--; i >= 0; i--) {
		fs[i] = string(c[i]);
	}
	OnOpenFile(fs);
	return true;
}

void ParLoader::OnOpenFile(const std::vector<string>& files) {
	if (!files.size()) return;
	droppedFiles = files;
	loadAsTrj = !!Particles::particleSz;
	FindImpId();

	useConn = true;
	hasConnCache = false;
	string cpt = droppedFiles[0] + ".conn";
	std::ifstream strm(cpt);
	if (strm.is_open()) {
		std::getline(strm, connCachePath);
		if (!!connCachePath.size()) {
			hasConnCache = true;
			struct stat st1, st2;
			stat(&droppedFiles[0][0], &st1);
			stat(&cpt[0], &st2);
			oldConnCache = ovwConnCache = st1.st_mtime > st2.st_mtime;
		}
	}
	else connCachePath.clear();


	useConnCache = hasConnCache;

	showDialog = true;
}

void ParLoader::FindImpId(bool force) {
	impId = -1;
	int id = 0;
	auto sz = droppedFiles[0].size();
	if (force) {
		if (!loadAsTrj) {
			for (auto imp : importers) {
				int id2 = 0;
				for (auto& pr : imp->funcs) {
					for (auto& s : pr.first) {
						if (EndsWith(droppedFiles[0], s)) {
							impId = id;
							funcId = id2;
							return;
						}
					}
					id2++;
				}
				id++;
			}
		}
		else {
			for (auto imp : importers) {
				int id2 = 0;
				for (auto& pr : imp->trjFuncs) {
					for (auto& s : pr.first) {
						if (EndsWith(droppedFiles[0], s)) {
							impId = id;
							funcId = id2;
							return;
						}
					}
					id2++;
				}
				id++;
			}
		}
	}
	else {
		for (auto imp : importers) {
			int id2 = 0;
			for (auto& pr : imp->funcs) {
				for (auto& s : pr.first) {
					if (EndsWith(droppedFiles[0], s)) {
						impId = id;
						funcId = id2;
						loadAsTrj = false;
						return;
					}
				}
				id2++;
			}
			id2 = 0;
			for (auto& pr : imp->trjFuncs) {
				for (auto& s : pr.first) {
					if (EndsWith(droppedFiles[0], s)) {
						impId = id;
						funcId = id2;
						loadAsTrj = true;
						return;
					}
				}
				id2++;
			}
			id++;
		}
	}
}