#include "parloader.h"
#include "Gromacs.h"
#include "Protein.h"
#include "vis/pargraphics.h"
#include "web/anweb.h"
#include "ui/icons.h"
#include "utils/rawvector.h"

int ParLoader::impId, ParLoader::funcId;
ParImporter* ParLoader::customImp;
bool ParLoader::loadAsTrj = false, ParLoader::additive = false;
int ParLoader::maxframes = 0;

std::vector<ParImporter*> ParLoader::importers;

bool ParLoader::showDialog = false;
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
			ostrm << "ok";
			continue;
		err:
			delete(imp);
		}
	}
}

bool ParLoader::DoOpen() {
	ParInfo info = {};
	info.path = droppedFiles[0].c_str();
	info.nameSz = PAR_MAX_NAME_LEN;

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
		return false;
	}

	Particles::particleSz = info.num;
	Particles::connSz = 0;
	Particles::particles_ResName = info.resname;
	Particles::particles_Name = info.name;
	Particles::particles_Pos = (Vec3*)info.pos;
	Particles::particles_Vel = (Vec3*)info.vel;
	Particles::boundingBox = *((Vec3*)info.bounds);
	Particles::particles_Col = new byte[info.num];
	Particles::particles_Rad = new float[info.num];
	Particles::particles_Res = new Int2[info.num];

	auto rs = rawvector<ResidueList, uint>(Particles::residueLists, Particles::residueListSz);
	auto rsv = rawvector<Residue, uint>(Particles::residueLists->residues, Particles::residueLists->residueSz);
	auto cn = rawvector<Int2, uint>(Particles::particles_Conn, Particles::connSz);

	uint64_t currResNm = -1;
	uint16_t currResId = -1;
	ResidueList* trs = 0;
	Residue* tr = 0;
	Vec3 vec;

	uint lastOff = 0, lastCnt = 0;

	for (uint i = 0; i < info.num; i++) {
		auto id1 = info.name[i * PAR_MAX_NAME_LEN];
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
			tr->offset_b = Particles::connSz;
			tr->name = std::to_string(resId);
			tr->type = AminoAcidType((char*)&resNm);
			tr->cnt = 0;
			tr->cnt_b = 0;
			currResId = resId;
		}
		Vec3 vec = *((Vec3*)(&info.pos[i * 3]));
		for (uint j = 0; j < lastCnt + tr->cnt; j++) {
			Vec3 dp = Particles::particles_Pos[lastOff + j] - vec;
			auto dst = glm::length2(dp);
			if (dst < 0.0625) { //2.5A
				auto id2 = Particles::particles_Name[(lastOff + j) * PAR_MAX_NAME_LEN];
				float bst = VisSystem::_bondLengths[id1 + (id2 << 16)];
				if (dst < bst) {
					cn.push(Int2(i, lastOff + j));
					tr->cnt_b++;
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

	Particles::UpdateBufs();
	Particles::GenTexBufs();
	Protein::Refresh();
	ParGraphics::UpdateDrawLists();
	return true;
}

bool ParLoader::DoOpenAnim() {
	auto& anm = Particles::anim;
	anm.reading = true;

	TrjInfo info = {};
	info.first = droppedFiles[0].c_str();
	info.parNum = Particles::particleSz;
	info.maxFrames = maxframes;

	if (impId > -1) importers[impId]->trjFuncs[funcId].second(&info);

	if (!info.frames) return false;

	anm.frameCount = info.frames;
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

	anm.reading = false;
	return true;
}

void ParLoader::DrawOpenDialog() {
	if (!showDialog) return;
	UI::IncLayer();
	Engine::DrawQuad(0, 0, (float)Display::width, (float)Display::height, black(0.7f));

	float woff = Display::width*0.5f - 200 - _impPos / 2;
	float hoff = Display::height * 0.5f - 150;

	if (_showImp || _impPos > 0) {
		Engine::DrawQuad(woff + 400, hoff, _impPos, 300, white(0.8f, 0.1f));
		Engine::PushStencil(woff + 400, hoff, _impPos, 300);
		UI::Label(woff + 402, hoff, 12, "Choose Importer", white());
		
		Engine::PopStencil();
	}
	_impPos = _showImp? min(_impPos + 800 * Time::delta, 100.0f) : max(_impPos - 800 * Time::delta, 0.0f);

	Engine::DrawQuad(woff, hoff, 400, 300, white(0.8f, 0.15f));
	Engine::DrawQuad(woff, hoff, 400, 16, white(0.9f, 0.1f));
	UI::Label(woff + 2, hoff, 12, loadAsTrj ? "Load Trajectory" : "Load Configuration", white());
	UI::Label(woff + 2, hoff + 17, 12, "File(s)", white());
	string nm = droppedFiles[0];
	if (Engine::Button(woff + 60, hoff + 17, 339, 16, white(1, 0.4f)) == MOUSE_RELEASE) {

	}
	UI::Label(woff + 62, hoff + 17, 12, nm, white(0.7f), 326);
	UI::Texture(woff + 383, hoff + 17, 16, 16, Icons::browse);
	
	UI::Label(woff + 2, hoff + 34, 12, "Importer", white(), 326);
	if (impId > -1)
		UI::Label(woff + 60, hoff + 34, 12, importers[impId]->name + " (" + importers[impId]->sig + ")", white(0.5f), 326);
	if (Engine::Button(woff + 339, hoff + 34, 60, 16, white(1, 0.4f), _showImp ? "<<" : ">>", 12, white(), true) == MOUSE_RELEASE) {
		_showImp = !_showImp;
	}
	
	//if (Particles::particleSz) {
		loadAsTrj = Engine::Toggle(woff + 1, hoff + 17 * 3, 16, Icons::checkbox, loadAsTrj, white(), ORIENT_HORIZONTAL);
		UI::Label(woff + 30, hoff + 17 * 3, 12, "As Trajectory", white(), 326);
		additive = Engine::Toggle(woff + 201, hoff + 17 * 3, 16, Icons::checkbox, additive, white(), ORIENT_HORIZONTAL);
		UI::Label(woff + 230, hoff + 17 * 3, 12, "Additive", white(), 326);
	//}
	UI::Label(woff + 2, hoff + 17 * 4, 12, "Options", white(), 326);


	string line = "";
	if (loadAsTrj) line += "-trj ";
	if (additive) line += "-a";
	if (maxframes > 0) line += "-n" + std::to_string(maxframes) + " ";
	UI::Label(woff + 2, hoff + 300 - 17 * 2, 12, "Command line : " + line, white(), 326);
	if (Engine::Button(woff + 300, hoff + 283, 48, 16, yellow(1, 0.4f), "Cancel", 12, white(), true) == MOUSE_RELEASE) {
		showDialog = false;
	}
	if (Engine::Button(woff + 350, hoff + 283, 49, 16, white(0.4f), "Load", 12, white(), true) == MOUSE_RELEASE) {
		DoOpen();
		showDialog = false;
	}
}

#define EndsWith(s, s2) s.substr(sz - s2.size()) == s2

bool ParLoader::OnDropFile(int i, const char** c) {
	if (AnWeb::drawFull) return false;
	droppedFiles.resize(i);
	for (i--; i >= 0; i--) {
		droppedFiles[i] = string(c[i]);
	}
	if (!Particles::particleSz) {
		impId = -1;
		int id = 0;
		auto sz = droppedFiles[0].size();
		for (auto imp : importers) {
			int id2 = 0;
			for (auto& pr : imp->funcs) {
				for (auto& s : pr.first) {
					if (EndsWith(droppedFiles[0], s)) {
						impId = id;
						funcId = id2;
						goto found;
					}
				}
				id2++;
			}
			id++;
		}
	}
	found:
	showDialog = true;
	return true;
}