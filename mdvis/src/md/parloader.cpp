#include "parloader.h"
#include "Gromacs.h"
#include "Protein.h"
#include "vis/pargraphics.h"
#include "parmenu.h"
#include "GenericSSV.h"
#include "web/anweb.h"
#include "ui/icons.h"
#include "ui/popups.h"
#include "ui/ui_ext.h"
#include <iomanip>

#define EndsWith(s, s2) sz > s2.size() && s.substr(sz - s2.size()) == s2

int ParLoader::impId, ParLoader::funcId;
ParImporter* ParLoader::customImp;
bool ParLoader::loadAsTrj = false, ParLoader::additive = false;
uint ParLoader::frameskip = 1;
int ParLoader::maxframes = -1;
bool ParLoader::useConn, ParLoader::useConnCache, ParLoader::hasConnCache, ParLoader::oldConnCache, ParLoader::ovwConnCache;
std::string ParLoader::connCachePath;

bool ParLoader::isSrv = false, ParLoader::srvusepass = false;
std::string ParLoader::srvuser, ParLoader::srvhost;
int ParLoader::srvport;
std::string ParLoader::srvkey, ParLoader::srvpass;
SSH ParLoader::srv;

std::vector<ParImporter> ParLoader::importers;
std::vector<std::string> ParLoader::exts;

bool ParLoader::showDialog = false, ParLoader::busy = false, ParLoader::fault = false, ParLoader::directLoad = false;
bool ParLoader::parDirty = false, ParLoader::trjDirty = false;
float* ParLoader::loadProgress = 0, *ParLoader::loadProgress2 = 0;
uint16_t* ParLoader::loadFrames = 0;
std::string ParLoader::loadName;
std::vector<std::string> ParLoader::droppedFiles;

bool ParLoader::_showImp = false;
float ParLoader::_impPos = 0, ParLoader::_impScr = 0;

void ParLoader::Init() {
	ChokoLait::dropFuncs.push_back(OnDropFile);

	std::ifstream strm(IO::path + ".srvinfo");
	if (strm) strm >> srvusepass >> srvuser >> srvhost >> srvport >> srvkey;
	else {
		srvusepass = false;
		srvuser = "username";
		srvhost = "host";
		srvport = 22;
		srvkey = "~/.ssh/id_rsa.pub";
	}
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

std::string _RMSP(std::string s) {
	std::string res;
	res.reserve(s.size());
	for (auto& c : s) {
		if (c != ' ') res += c;
	}
	return res;
}

void ParLoader::Scan() {
	std::string fd = IO::path + "bin/importers/";
	std::vector<std::string> fds;
	IO::GetFolders(fd, &fds);
	exts.clear();
	for (auto& f : fds) {
		std::cout << "Import: " << f << std::endl;
		std::ofstream ostrm(fd + f + "/import.log");
		std::ifstream strm(fd + f + "/config.txt");
		if (strm.is_open()) {
			ParImporter imp = ParImporter();
			std::string s, libname;
			while (std::getline(strm, s)) {
				auto lc = s.find_first_of('=');
				if (lc != std::string::npos) {
					auto tp = s.substr(0, lc);
					auto vl = s.substr(lc + 1);
					if (tp == "name") imp.name = vl;
					else if (tp == "signature") imp.sig = vl;
					else if (tp == "file") {
						imp.lib = new DyLib(fd + f + OSFD + vl + LIBEXT);
						if (!imp.lib) {
							ostrm << "Importer lib file not found!";
						}
					}
				}
				else {
					if (!imp.lib) {
						ostrm << "Importer lib file must be defined before configurations!";
					}
					s = _RMSP(s);
					auto lc = s.find('{');
					if (lc != std::string::npos) {
						auto tp = s.substr(0, lc);
						if (tp == "configuration") {
							std::getline(strm, s);
							s = _RMSP(s);
							ParImporter::Func pr;
							int vlc = 0;
							while (s != "}") {
								auto lc = s.find_first_of('=');
								if (lc == std::string::npos) continue;
								auto tp = s.substr(0, lc);
								auto vl = s.substr(lc + 1);
								if (tp == "func") {
									if (!(pr.func = (ParImporter::loadsig)imp.lib->GetSym(vl))) {
										ostrm << "Importer function \"" << vl << "\" not found!";
									}
									pr.type = ParImporter::Func::FUNC_TYPE::CONFIG;
									vlc++;
								}
								else if (tp == "exts") {
									auto ob = vl.find('[');
									auto cb = vl.find(']');
									pr.exts = string_split(vl.substr(ob + 1, cb - ob - 1), ',');
									vlc++;
								}
								std::getline(strm, s);
								s = _RMSP(s);
							}
							if (vlc == 2) {
								imp.funcs.push_back(pr);
							}
						}
						else if (tp == "frame") {
							std::getline(strm, s);
							s = _RMSP(s);
							ParImporter::Func pr;
							int vlc = 0;
							while (s != "}") {
								auto lc = s.find_first_of('=');
								if (lc == std::string::npos) continue;
								auto tp = s.substr(0, lc);
								auto vl = s.substr(lc + 1);
								if (tp == "func") {
									if (!(pr.frmFunc = (ParImporter::loadfrmsig)imp.lib->GetSym(vl))) {
										ostrm << "Importer function \"" << vl << "\" not found!";
									}
									vlc++;
								}
								else if (tp == "exts") {
									auto ob = vl.find('[');
									auto cb = vl.find(']');
									pr.exts = string_split(vl.substr(ob + 1, cb - ob - 1), ',');
									vlc++;
								}
								std::getline(strm, s);
								s = _RMSP(s);
							}
							if (vlc == 2) {
								imp.funcs.push_back(pr);
							}
						}
						else if (tp == "trajectory") {
							std::getline(strm, s);
							s = _RMSP(s);
							ParImporter::Func pr;
							int vlc = 0;
							while (s != "}") {
								auto lc = s.find_first_of('=');
								if (lc == std::string::npos) continue;
								auto tp = s.substr(0, lc);
								auto vl = s.substr(lc + 1);
								if (tp == "func") {
									if (!(pr.trjFunc = (ParImporter::loadtrjsig)imp.lib->GetSym(vl))) {
										ostrm << "Importer function \"" << vl << "\" not found!";
									}
									vlc++;
								}
								else if (tp == "exts") {
									auto ob = vl.find('[');
									auto cb = vl.find(']');
									pr.exts = string_split(vl.substr(ob + 1, cb - ob - 1), ',');
									vlc++;
								}
								std::getline(strm, s);
								s = _RMSP(s);
							}
							if (vlc == 2) {
								imp.funcs.push_back(pr);
							}
						}
					}
				}
			}
			if (!imp.name.size() || !imp.sig.size() || !imp.lib || !imp.funcs.size()) {
				ostrm << "Config contents incomplete!";
			}
			importers.push_back(imp);
			for (auto& a : imp.funcs) {
				exts.insert(exts.end(), a.exts.begin(), a.exts.end());
			}
			ostrm << "ok";
			continue;
		}
	}
}

void ParLoader::SrvConnect() {
	SaveSrvInfo();
	SSHConfig config;
	config.user = srvuser;
	config.ip = srvhost;
	config.port = srvport;
	config.auth = srvusepass ? SSH_Auth::PASSWORD : SSH_Auth::PUBKEY;
	if (!srvusepass) {
		config.keyPath1 = IO::ResolveUserPath(srvkey);
		config.keyPath2 = config.keyPath1.substr(0, config.keyPath1.find_last_of('.'));
	}
	config.pw = srvpass;
	srv = SSH::Connect(config);
	if (srv.ok) Debug::Message("ParLoader", "Connected to host");
	else Debug::Warning("ParLoader", "Failed to connect to host!");
}

void ParLoader::SrvDisconnect() {
	if (srv.ok) {
		srv.Disconnect();
		Debug::Message("ParLoader", "Disconnected from host");
	}
}

void ParLoader::SaveSrvInfo() {
	std::ofstream strm(IO::path + ".srvinfo");
	strm << srvusepass << "\n"
	<< srvuser << "\n"
	<< srvhost << "\n"
	<< srvport << "\n"
	<< srvkey;
}

#define ISNUM(c) (c >= '0' && c <= '9')
void ParLoader::ScanFrames(const std::string& first) {
	if (Particles::anim.frameCount > 1) return;
	if (maxframes == 0 || maxframes == 1) return;
	auto ps = first.find_last_of('/');
	auto nm = first.substr(ps + 1);
	int n1 = -1, n2 = 0;
	for (uint a = 0; a < nm.size() - 1; ++a)  {
		if (nm[a + 1] == '.' && ISNUM(nm[a])) {
			n1 = a;
			break;
		}
	}
	if (n1 == -1) return;
	for (int a = n1 - 1; a >= 0; --a) {
		if (!ISNUM(nm[a])) {
			n2 = a;
			break;
		}
	}

	int st = std::stoi(nm.substr(n2 + 1, n1 - n2));
	auto nm1 = first.substr(0, ps + 2 + n2);
	auto nm2 = nm.substr(n1 + 1);

	uint i = 0;
	for (auto& p : importers) {
		int id2 = 0;
		for (auto& pr : p.funcs) {
			if ((pr.type == ParImporter::Func::FUNC_TYPE::FRAME) && 
				(std::find(pr.exts.begin(), pr.exts.end(), nm2) != pr.exts.end())) {
				Particles::anim.impId = i;
				Particles::anim.funcId = id2;
				goto found;
			}
			id2++;
		}
		i++;
	}
	Debug::Warning("ParLoader", "Cannot find frame loader for extension \"" + nm2 + "\"! Trying to use configuration loader...");
	{
		int id2 = 0;
		for (auto& pr : importers[impId].funcs) {
			if (pr.type == ParImporter::Func::FUNC_TYPE::FRAME){
				Particles::anim.impId = impId;
				Particles::anim.funcId = id2;
				goto found;
			}
			id2++;
		}
	}
	Debug::Warning("ParLoader", "Configuration loader does not have a frame loader!");
	return;
	found:

	uint frms = 0;
	std::vector<std::string> nms;
	do {
		std::stringstream sstrm;
		sstrm << std::setw(n1 - n2) << std::setfill('0') << st;
		std::string nm = nm1 + sstrm.str() + nm2;
		if (isSrv) {
			if (!srv.HasFile(nm)) break;
		}
		else if (!IO::HasFile(nm)) break;
		nms.push_back(nm);
		frms++;
		st += frameskip;
	} while (frms != maxframes);
	if (frms <= 1) return;
	Particles::anim.AllocFrames(frms);
	for (uint f = 0; f < frms; ++f) {
		//Particles::anim.status[f] = Particles::AnimData::FRAME_STATUS::UNLOADED;
		Particles::anim.paths[f] = nms[f];
	}

}

void ParLoader::DoOpen() {
	std::replace(droppedFiles[0].begin(), droppedFiles[0].end(), '\\', '/');
	std::string nm = droppedFiles[0].substr(droppedFiles[0].find_last_of('/') + 1);
	auto path = droppedFiles[0];

	busy = true;
	ParInfo info = {};
	loadProgress = &info.progress;
	loadProgress2 = &info.trajectory.progress;
	loadFrames = &info.trajectory.frames;

	if (isSrv) {
		*loadProgress = 0.001f;
		loadName = "Downloading";
		path = IO::path + "tmp/" + nm;
		srv.GetFile(droppedFiles[0], path);
	}

	info.path = info.trajectory.first = path.c_str();
	info.nameSz = PAR_MAX_NAME_LEN;
	
	loadName = "Reading file(s)";
	VisSystem::SetMsg("Reading " + nm);

	auto t = milliseconds();

	//
	info.trajectory.maxFrames = maxframes;
	info.trajectory.frameSkip = frameskip;

	try {
		if (impId > -1) {
			Debug::Message("ParLoader", "Running importer \"" + importers[impId].name + "\"");
			if (!importers[impId].funcs[funcId].func(&info)) {
				if (!info.error[0]) {
					Debug::Warning("ParLoader", "Unspecified importer error!");
					VisSystem::SetMsg("Unspecified import error", 2);
				}
				else {
					Debug::Warning("ParLoader", "Importer error: " + std::string(info.error));
					VisSystem::SetMsg(info.error, 2);
				}
				if (isSrv) remove(path.c_str());
				busy = false;
				fault = true;
				return;
			}
		}
		else {
			Debug::Warning("ParLoader", "No importer set!");
			if (isSrv) remove(path.c_str());
			busy = false;
			fault = true;
			return;
		}
	}
	catch (char* c) {
		Debug::Warning("ParLoader", "Importer exception: " + std::string(c));
		VisSystem::SetMsg("Importer threw " + std::string(c), 2);
		if (isSrv) remove(path.c_str());
		busy = false;
		fault = true;
		return;
	}

	if (isSrv) remove(path.c_str());

	loadProgress = 0;
	*loadProgress2 = 0;
	loadFrames = nullptr;

	Particles::Resize(info.num);
	memcpy(&Particles::names[0], info.name, info.num * PAR_MAX_NAME_LEN);
	memcpy(&Particles::resNames[0], info.resname, info.num * PAR_MAX_NAME_LEN);
	Particles::poss = (glm::dvec3*)info.pos;
	Particles::vels = (glm::dvec3*)info.vel;
	memcpy(&Particles::types[0], info.type, info.num * sizeof(short));
	memcpy(Particles::boundingBox, info.bounds, 6 * sizeof(double));
	if (!VisSystem::currentSavePath.size())
		ParGraphics::rotCenter = Vec3(info.bounds[0] + info.bounds[1], 
		info.bounds[2] + info.bounds[3], 
		info.bounds[4] + info.bounds[5]) * 0.5f;

	auto& conn = Particles::conns;

	uint64_t currResNm = -1;
	uint16_t currResId = -1;
	ResidueList* trs = 0;
	Residue* tr = 0;
	Vec3 vec;

	uint lastOff = 0, lastCnt = 0;

	std::ifstream* strm = 0;

	if (useConn) {
		if (useConnCache && hasConnCache && !ovwConnCache) {
			strm = new std::ifstream(path + ".conn", std::ios::binary);
			char buf[100];
			strm->getline(buf, 100, '\n');
			strm->read((char*)(&conn.cnt), 4);
			conn.ids.resize(conn.cnt);
			strm->read((char*)&conn.ids[0], conn.cnt * sizeof(Int2));
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
	for (uint i = 0; i < info.num; ++i)  {
		info.progress = i * 1.f / info.num;
		auto id1 = info.type[i];//info.name[i * PAR_MAX_NAME_LEN];
		auto& resId = info.resId[i];
		uint64_t resNm = *((uint64_t*)(&info.resname[i * PAR_MAX_NAME_LEN])) & 0x000000ffffffffff;

		if (currResNm != resNm) {
			Particles::residueLists.push_back(ResidueList());
			Particles::residueListSz++;
			trs = &Particles::residueLists.back();
			trs->name = std::string(info.resname + i * PAR_MAX_NAME_LEN, 5);
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
			trs->residues.push_back(Residue());
			trs->residueSz++;
			tr = &trs->residues.back();
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
				tr->offset_b = conn.cnt;
				tr->cnt_b = 0;
			}
		}
		auto vec = *((glm::dvec3*)(&info.pos[i * 3]));
		int cnt = int(lastCnt + tr->cnt);
		if (useConn && (!useConnCache || !hasConnCache || ovwConnCache)) {
			for (int j = 0; j < cnt; ++j)  {
				Vec3 dp = Particles::poss[lastOff + j] - vec;
				if (fabsf(dp.x) < 0.25f && fabsf(dp.y) < 0.25f && fabsf(dp.z) < 0.25f) {
					auto dst = glm::length2(dp);
					uint32_t id2 = info.type[lastOff + j];
					float bst = VisSystem::_bondLengths[id1 + (id2 << 16)];
					if (!bst) bst = VisSystem::_defBondLength;
					if (dst < bst) {
						conn.ids.push_back(Int2(i, lastOff + j));
						conn.cnt++;
						tr->cnt_b++;
					}
				}
			}
		}
		for (byte b = 0; b < Particles::defColPalleteSz; ++b)  {
			if (id1 == Particles::defColPallete[b]) {
				Particles::colors[i] = b;
				break;
			}
		}

		float rad = VisSystem::radii[id1][1];

		Particles::radii[i] = rad;
		Particles::ress[i] = Int2(Particles::residueListSz - 1, trs->residueSz - 1);

		tr->cnt++;
	}

	if (strm) delete(strm);

	if (useConnCache && (!hasConnCache || ovwConnCache)) {
		std::ofstream ostrm(path + ".conn", std::ios::binary);
		ostrm << __DATE__ << " " << __TIME__ << "\n";
		_StreamWrite(&conn.cnt, &ostrm, 4);
		_StreamWrite(&conn.ids[0], &ostrm, conn.cnt * sizeof(Int2));
		_StreamWrite("XX", &ostrm, 2);
		for (uint a = 0; a < Particles::residueListSz; ++a)  {
			auto& rsl = Particles::residueLists[a];
			for (uint b = 0; b < rsl.residueSz; ++b)  {
				_StreamWrite(&rsl.residues[b].offset_b, &ostrm, sizeof(uint));
				_StreamWrite(&rsl.residues[b].cnt_b, &ostrm, sizeof(ushort));
			}
		}
	}

	delete[](info.name);
	delete[](info.resname);
	delete[](info.type);
	delete[](info.resId);

	auto& anm = Particles::anim;
	anm.Clear();
	if (info.trajectory.frames > 0) {
		auto& trj = info.trajectory;
		anm.reading = true;

		anm.AllocFrames(trj.frames);
		for (uint16_t i = 0; i < trj.frames; ++i)  {
			anm.poss[i].resize(info.num);
			memcpy(&anm.poss[i][0], trj.poss[i], info.num * sizeof(glm::dvec3));
			delete[](trj.poss[i]);
			anm.vels[i].resize(info.num);
			if (trj.vels) {
				memcpy(&anm.vels[i][0], trj.vels[i], info.num * sizeof(glm::dvec3));
				delete[](trj.vels[i]);
			}
			anm.status[i] = Particles::AnimData::FRAME_STATUS::LOADED;
		}
		delete[](trj.poss);
		if (trj.vels) delete[](trj.vels);
		if (trj.bounds) {
			anm.bboxs.resize(6*trj.frames);
			for (uint16_t i = 0; i < trj.frames; ++i)  {
				memcpy(&anm.bboxs[i*6], trj.bounds[i], 6*sizeof(double));
			}
			delete[](trj.bounds);
		}
		anm.reading = false;

		anm.maxFramesInMem = 10000000;
	}
	else {
		anm.frameCount = 1;
	}

	VisSystem::SetMsg("Loaded file(s) in " + std::to_string((milliseconds() - t)*0.001f).substr(0, 5) + "s");
	ParMenu::SaveRecents(droppedFiles[0]);
	Particles::cfgFile = droppedFiles[0];

	frameskip = FindNextOff(droppedFiles[0]);
	ScanFrames(droppedFiles[0]);

	if (Particles::anim.poss.size()) {
		Particles::anim.poss[0].resize(info.num);
		memcpy(&Particles::anim.poss[0][0], Particles::poss, info.num * sizeof(glm::dvec3));
		delete[](Particles::poss);
		Particles::poss = &Particles::anim.poss[0][0];
		if (info.vel) {
			Particles::anim.vels[0].resize(info.num);
			memcpy(&Particles::anim.vels[0][0], Particles::vels, info.num * sizeof(glm::dvec3));
			delete[](Particles::vels);
			Particles::vels = &Particles::anim.vels[0][0];
		}
		Particles::anim.status[0] = Particles::AnimData::FRAME_STATUS::LOADED;
		Particles::anim.dirty = true;
	}

	parDirty = true;
	busy = false;
	fault = false;

	//temp
	if (!isSrv)
		anm.maxFramesInMem = anm.frameCount;
}

void ParLoader::DoOpenAnim() {
	busy = true;
	auto& anm = Particles::anim;
	anm.reading = true;
	anm.Clear();
	
	std::replace(droppedFiles[0].begin(), droppedFiles[0].end(), '\\', '/');
	std::string nm = droppedFiles[0].substr(droppedFiles[0].find_last_of('/') + 1);
	auto path = droppedFiles[0];

	TrjInfo info = {};
	info.parNum = Particles::particleSz;
	info.maxFrames = maxframes;
	loadFrames = &info.frames;
	loadProgress = &info.progress;

	if (isSrv) {
		*loadProgress = 0.001f;
		loadName = "Downloading";
		path = IO::path + "tmp/" + nm;
		srv.GetFile(droppedFiles[0], path);
	}

	info.first = path.c_str();

	Debug::Message("ParLoader", "Running importer \"" + importers[impId].name + "\"");
	importers[impId].funcs[funcId].trjFunc(&info);

	if (!info.frames) {
		busy = false;
		fault = true;
	}

	Engine::AcquireLock(1);

	anm.AllocFrames(info.frames);
	for (uint16_t i = 0; i < info.frames; ++i)  {
		anm.poss[i].resize(info.parNum);
		memcpy(&anm.poss[i][0], info.poss[i], info.parNum * sizeof(glm::dvec3));
		delete[](info.poss[i]);
		anm.vels[i].resize(info.parNum);
		if (info.vels) {
			memcpy(&anm.vels[i][0], info.poss[i], info.parNum * sizeof(glm::dvec3));
			delete[](info.poss[i]);
		}
		anm.status[i] = Particles::AnimData::FRAME_STATUS::LOADED;
	}
	anm.maxFramesInMem = 10000000;
	delete[](info.poss);
	if (info.vels) delete[](info.vels);

	Particles::trjFile = droppedFiles[0];
	anm.reading = false;
	anm.dirty = true;
	busy = false;
	fault = false;
	
	Engine::ReleaseLock();
}

void ParLoader::OpenFrame(uint f, const std::string& path) {
	std::thread(OpenFrameNow, f, path).detach();
}

void ParLoader::OpenFrameNow(uint f, std::string path) {
	while (busy){}
	Debug::Message("ParLoader", "Load frame " + std::to_string(f));
	std::flush(std::cout);
	using FS = Particles::AnimData::FRAME_STATUS;
	busy = true;
	auto& anm = Particles::anim;
	anm.reading = true;
	anm.status[f] = FS::READING;

	if (isSrv) {
		if (srv.ok) {
			auto pl = path.find_last_of('/');
			auto nm = path.substr(pl + 1);
			auto p2 = IO::path + "tmp/" + nm;
			srv.GetFile(path, p2);
			path = p2;
		}
		else {
			anm.reading = false;
			busy = false;
			anm.status[f] = FS::BAD;
			return;
		}
	}

	auto& pos = anm.poss[f];
	auto& vel = anm.vels[f];
	pos.resize(Particles::particleSz);
	vel.resize(Particles::particleSz);
	FrmInfo info(path.c_str(), Particles::particleSz, &pos[0][0], &vel[0][0]);
	Debug::Message("ParLoader", "Running importer \"" + importers[anm.impId].name + "\"");
	fault = !importers[anm.impId].funcs[anm.funcId].frmFunc(&info);
	if (fault) {
		ErrorView::Message msg;
		msg.name = "System";
		msg.severe = true;
		msg.msg.resize(1, "failed to load frame " + std::to_string(f) + ": " + std::string(info.error));
		ErrorView::execMsgs.push_back(msg);
		anm.status[f] = FS::BAD;
	}
	else anm.status[f] = FS::LOADED;

	if (!ParLoader::impId) {
		for (int a = 0, s = GenericSSV::_attrs.size(); a < s; a++) {
			auto& tr = GenericSSV::_attrs[a];
			if (!!tr.size()) {
				Particles::attrs[a]->timed = true;
				Particles::attrs[a]->Get(f) = tr;
				tr.clear();
			}
		}
	}

	anm.reading = false;
	if (isSrv) remove(path.c_str());
	busy = false;
}

void ParLoader::DrawOpenDialog() {
	if (!showDialog) return;
	UI::IncLayer();
	UI::Quad(0, 0, static_cast<float>(Display::width), static_cast<float>(Display::height), black(0.7f));

	float woff = roundf(Display::width*0.5f - 200 - _impPos / 2);
	float hoff = roundf(Display::height * 0.5f - 150);

	if (_showImp || _impPos > 0) {
		UI::Quad(woff + 400, hoff, _impPos, 300, white(0.8f, 0.1f));
		Engine::PushStencil(woff + 400, hoff, _impPos, 300);
		UI::Label(woff + 402, hoff, 12, "Choose Importer", white());
		uint i = 1;
		for (auto& p : importers) {
			if (Engine::Button(woff + 401, hoff + 17 * i, 298, 16, (impId == i - 1) ? Vec4(0.5f, 0.4f, 0.2f, 1) : white(0.4f), p.name, 12, white()) == MOUSE_RELEASE) {
				impId = i - 1;
				funcId = 0;
				int id2 = 0;
				auto sz = droppedFiles[0].size();
				for (auto& pr : p.funcs) {
					for (auto& s : pr.exts) {
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
	_impPos = _showImp ? min(_impPos + 800 * Time::delta, 100.f) : max(_impPos - 800 * Time::delta, 0.f);

	UI::Quad(woff, hoff, 400, 300, white(0.8f, 0.15f));
	UI::Quad(woff, hoff, 400, 16, white(0.9f, 0.1f));
	UI::Label(woff + 2, hoff, 12, loadAsTrj ? "Import Trajectory" : "Import Configuration", white());
	hoff += 17;
	
	static bool hsf = true;
	static std::string ff = "";

	UI2::sepw = 0.33f;
	static std::string opts[] = { "Local", "Remote", "" };
	static uint issv = isSrv;
	static Popups::DropdownItem di(&issv, &opts[0]);
	UI2::Dropdown(woff + 2, hoff, 170, "Location", di);
	if (isSrv != !!issv) {
		isSrv = !!issv;
		ff = "";
	}
	if (isSrv) {
		UI2::sepw = 0.25f;
		static std::string opts2[] = { "Public Key", "Password", "" };
		static uint ispw = srvusepass;
		static Popups::DropdownItem di2(&ispw, &opts2[0]);
		srvusepass = !!ispw;
		UI2::Dropdown(woff + 180, hoff, 120, "Auth", di2);

		if (srv.ok) {
			if (Engine::Button(woff + 319, hoff, 80, 16, red(1, 0.5f), "Disconnect", 12, white(), true) == MOUSE_RELEASE) {
				SrvDisconnect();
			}
		}
		else {
			if (Engine::Button(woff + 319, hoff, 80, 16, green(1, 0.5f), "Connect", 12, white(), true) == MOUSE_RELEASE) {
				SrvConnect();
				ff = "";
			}
		}

		hoff += 17;
		srvuser = UI2::EditText(woff + 2, hoff, 130, "Host", srvuser, !srv.ok);
		UI2::sepw = 0.12f;
		srvhost = UI2::EditText(woff + 135, hoff, 150, "@", srvhost, !srv.ok);
		UI2::sepw = 0.2f;
		srvport = TryParse(UI2::EditText(woff + 290, hoff, 80, "-p", std::to_string(srvport), !srv.ok), 22);
		hoff += 17;
		UI2::sepw = 0.3f;
		if (srvusepass) {
			srvpass = UI2::EditPass(woff + 2, hoff, 250, "Password", srvpass, !srv.ok);
		}
		else {
			srvkey = UI2::EditText(woff + 2, hoff, 250, "Public Key", srvkey, !srv.ok);
			srvpass = "";
		}
	}
	else if (srv.ok) SrvDisconnect();
	hoff += 17;
	UI2::sepw = 0.1f;
	if (ff != droppedFiles[0]) {
		ff = droppedFiles[0];
		if (isSrv) {
			if (srv.ok)
				hsf = srv.HasFile(ff);
			else hsf = false;
		}
		else {
			hsf = IO::HasFile(ff);
		}
		if (hsf) FindImpId();
	}
	droppedFiles[0] = UI2::EditText(woff + 2, hoff, 381, "File", droppedFiles[0], true, hsf? white(1, 0.5f) : red(1, 0.5f));
	
	Engine::Button(woff + 383, hoff, 16, 16, Icons::browse);

	UI::Label(woff + 2, hoff + 17 * 2, 12, "Importer", white(), 326);
	if (impId > -1)
		UI::Label(woff + 60, hoff + 34, 12, importers[impId].name + " (" + importers[impId].sig + ")", white(0.5f), 326);
	if (Engine::Button(woff + 339, hoff + 34, 60, 16, white(1, 0.4f), _showImp ? "<<" : ">>", 12, white(), true) == MOUSE_RELEASE) {
		_showImp = !_showImp;
	}
	UI::Label(woff + 2, hoff + 17 * 3, 12, "Module", white(), 326);
	if (impId > -1) {
		auto& ii = importers[impId];
		uint i = 0;
		if (loadAsTrj) {
			for (auto& f : ii.funcs) {
				if (f.type == ParImporter::Func::FUNC_TYPE::CONFIG) continue;
				if (Engine::Button(woff + 60 + 50 * i, hoff + 17 * 3, 45, 16, (funcId == i) ? Vec4(0.5f, 0.4f, 0.2f, 1) : white(0.4f), f.exts[0], 12, white(), true) == MOUSE_RELEASE) {
					funcId = i;
				}
				i++;
			}
		}
		else {
			for (auto& f : ii.funcs) {
				if (f.type != ParImporter::Func::FUNC_TYPE::CONFIG) continue;
				if (Engine::Button(woff + 60 + 50 * i, hoff + 17 * 3, 45, 16, (funcId == i)? Vec4(0.5f, 0.4f, 0.2f, 1) : white(0.4f), f.exts[0], 12, white(), true) == MOUSE_RELEASE) {
					funcId = i;
				}
				i++;
			}
		}
	}
	auto l = Engine::Toggle(woff + 5, hoff + 17 * 4, 16, Icons::checkbox, loadAsTrj, white(), ORIENT_HORIZONTAL);
	if (l != loadAsTrj) {
		loadAsTrj = l;
		FindImpId(true);
	}
	UI::Label(woff + 34, hoff + 17 * 4, 12, "As Trajectory", white(), 326);
	additive = Engine::Toggle(woff + 201, hoff + 17 * 4, 16, Icons::checkbox, additive, white(), ORIENT_HORIZONTAL);
	UI::Label(woff + 230, hoff + 17 * 4, 12, "Additive", white(), 326);

	UI::Label(woff + 2, hoff + 17 * 5, 12, "Options", white(), 326);
	useConn = Engine::Toggle(woff + 5, hoff + 17 * 6, 16, Icons::checkbox, useConn, white(), ORIENT_HORIZONTAL);
	UI::Label(woff + 25, hoff + 17 * 6, 12, "Bonds", white(), 326);
	if (useConn) {
		useConnCache = Engine::Toggle(woff + 100, hoff + 17 * 6, 16, Icons::checkbox, useConnCache, white(), ORIENT_HORIZONTAL);
		UI::Label(woff + 123, hoff + 17 * 6, 12, "Use Bond Cache", white(), 326);
		if (useConnCache) {
			UI::Label(woff + 5, hoff + 17 * 7, 12, hasConnCache ? "Cache found" + (std::string)(oldConnCache ? "(outdated): " : ": ") + connCachePath : "No cache found", white(), 326);
			if (hasConnCache) {
				ovwConnCache = Engine::Toggle(woff + 300, hoff + 17 * 7, 16, Icons::checkbox, ovwConnCache, white(), ORIENT_HORIZONTAL);
				UI::Label(woff + 320, hoff + 17 * 7, 12, "Overwrite", white(), 326);
			}
		}
	}
	maxframes = TryParse(UI2::EditText(woff + 2, hoff + 17 * 8, 200, "Max Frames", std::to_string(maxframes)), 1000);
	/*
	std::string line = "";
	if (loadAsTrj) line += "-trj ";
	if (additive) line += "-a";
	if (maxframes > 0) line += "-n" + std::to_string(maxframes) + " ";
	UI::Label(woff + 2, hoff + 300 - 17 * 2, 12, "Command line : " + line, white(), 326);
	*/
	hoff = roundf(Display::height * 0.5f + 150 - 17);

	if (Engine::Button(woff + 300, hoff, 48, 16, yellow(1, 0.4f), "Cancel", 12, white(), true) == MOUSE_RELEASE) {
		showDialog = false;
		ff = "";
	}
	bool cn = hsf && (!isSrv || srv.ok);
	if (impId > -1 && (Engine::Button(woff + 350, hoff, 49, 16, white(0.4f), "Load", 12, white(cn? 1 : 0.2f), true) == MOUSE_RELEASE) && cn) {
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
		ff = "";
	}
	
	UI2::sepw = 0.5f;
}

bool ParLoader::OnDropFile(int i, const char** c) {
	if (AnWeb::drawFull) return false;
	std::vector<std::string> fs(i);
	for (i--; i >= 0; --i) {
		fs[i] = std::string(c[i]);
	}
	OnOpenFile(fs);
	return true;
}

void ParLoader::OnOpenFile(const std::vector<std::string>& files) {
	if (!files.size()) return;
	droppedFiles = files;
	loadAsTrj = !!Particles::particleSz;
	ParMenu::showSplash = false;
	FindImpId();
	//frameskip = FindNextOff(files[0]);

	useConn = true;
	hasConnCache = false;

	std::string cpt = droppedFiles[0] + ".conn";
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
	if (directLoad) {
		directLoad = false;
		ParMenu::showSplash = false;
		if (loadAsTrj) {
			std::thread td(DoOpenAnim);
			td.detach();
		}
		else {
			Particles::Clear();
			std::thread td(DoOpen);
			td.detach();
		}
	}
	else showDialog = true;
}

void ParLoader::FindImpId(bool force) {
	impId = -1;
	int id = 0;
	auto sz = droppedFiles[0].size();
	for (auto& imp : importers) {
		int id2 = 0;
		for (auto& pr : imp.funcs) {
			if (force || ((pr.type != ParImporter::Func::FUNC_TYPE::CONFIG) == loadAsTrj)) {
				for (auto& s : pr.exts) {
					if (EndsWith(droppedFiles[0], s)) {
						impId = id;
						funcId = id2;
						if (force) loadAsTrj = (pr.type != ParImporter::Func::FUNC_TYPE::CONFIG);
						return;
					}
				}
			}
			id2++;
		}
		id++;
	}
}

uint ParLoader::FindNextOff(std::string path) {
	auto ld = path.find_last_of('.');
	std::string ext = path.substr(ld);
	path = path.substr(0, ld);
	auto ls = path.find_last_of('/') + 1;
	std::string nm = path.substr(ls);
	path = path.substr(0, ls);
	auto fls = isSrv? srv.ListFiles(path) : IO::GetFiles(path, ext);
	auto sz = nm.size();
	auto off = sz - 1;
	while (nm[off] >= '0' && nm[off] <= '9') {
		uint mn = ~0U;
		auto nm2 = nm.substr(0, off);
		uint mv = (uint)std::atoi(&nm[off]);
		for (auto& f : fls) {
			if (!string_find(f, nm2)) {
				uint val = TryParse(f.substr(off), ~0U);
				if (val > mv && val < mn) {
					mn = val;
				}
			}
		}
		if (mn != ~0U) {
			return mn - mv;
		}
		off--;
	}
	return 1;
}