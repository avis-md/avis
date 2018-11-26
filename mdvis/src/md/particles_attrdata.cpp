#include "particles.h"
#include "utils/glext.h"

int Particles::attrdata::_ids = 0;

Particles::attrdata::attrdata(bool ro) : instanceId(++_ids), readonly(ro), timed(false), _timed(false) {
	diskFd = IO::path + "tmp/attr_" + std::to_string(instanceId) + "_";
	glGenBuffers(1, &buf);
	SetGLBuf<float>(buf, nullptr, particleSz);
	glGenTextures(1, &texBuf);
	glBindTexture(GL_TEXTURE_BUFFER, texBuf);
	glTexBuffer(GL_TEXTURE_BUFFER, GL_R32F, buf);
	glBindTexture(GL_TEXTURE_BUFFER, 0);
	ApplyParCnt();
	ApplyFrmCnt();
}

Particles::attrdata::~attrdata() {
	glDeleteBuffers(1, &buf);
	glDeleteTextures(1, &texBuf);
}

std::vector<double>& Particles::attrdata::Get(uint frm) {
	if (!frm || !timed) return data;
	else {
		if (status[frm] == FRAME_STATUS::UNLOADED) {
			FromDisk(frm - 1);
		}
		return dataAll[frm - 1];
	}
}

void Particles::attrdata::Set(uint frm) {
	if (!!frm && !!Get(frm).size()) {
		status[frm - 1] = FRAME_STATUS::WAITWRITE;
	}
}

void Particles::attrdata::ApplyParCnt() {
	if (!particleSz) return;
	SetGLBuf<float>(buf, nullptr, particleSz);
}

void Particles::attrdata::ApplyFrmCnt() {
	if (Particles::anim.frameCount < 2) return;
	dataAll.resize(Particles::anim.frameCount-1);
	status.resize(readonly? Particles::anim.frameCount : Particles::anim.frameCount-1);
}

void Particles::attrdata::Update() {
	if (_timed != timed) {
		if (Particles::anim.frameCount < 2) timed = false;
		else {
			_timed = timed;
			if (!timed && Particles::anim.frameCount > 1) {
				std::vector<std::vector<double>>(Particles::anim.frameCount - 1).swap(dataAll);
			}
		}
	}

	if (timed) {
		if (readonly) {
			for (uint f = 0; f < Particles::anim.frameCount; ++f) {
				if (status[f] != FRAME_STATUS::UNLOADED) {
					if (anim.status[f] == animdata::FRAME_STATUS::UNLOADED) {
						std::vector<double>().swap(Get(f));
						status[f] = FRAME_STATUS::UNLOADED;
					}
				}
			}
		}
		else {
			for (uint f = 0; f < Particles::anim.frameCount-1; ++f) {
				switch (status[f]) {
				case FRAME_STATUS::WAITWRITE:
					ToDisk(f);
					status[f] = FRAME_STATUS::LOADED;
					break;
				case FRAME_STATUS::LOADED:
					if (anim.status[f+1] == animdata::FRAME_STATUS::UNLOADED) {
						std::vector<double>().swap(Get(f + 1));
						status[f] = FRAME_STATUS::UNLOADED;
					}
					break;
				case FRAME_STATUS::UNLOADED:
					if (anim.status[f+1] == animdata::FRAME_STATUS::LOADED) {
						FromDisk(f);
						status[f] = FRAME_STATUS::LOADED;
					}
					break;
				default:
					break;
				}
			}
		}
	}
}

void Particles::attrdata::Seek(uint frm) {
	auto& dt = Get(frm);
	auto sz = dt.size();
	if (!sz) return;
	SetGLSubBuf<>(buf, dt.data(), particleSz);
	dirty = false;
}

void Particles::attrdata::Clear() {
	timed = false;
	std::vector<double>().swap(data);
	std::vector<std::vector<double>>().swap(dataAll);
}

void Particles::attrdata::ToDisk(int i) {
	std::ofstream strm(diskFd + std::to_string(i), std::ios::binary);
	auto& d = dataAll[i];
	strm.write((char*)d.data(), d.size() * sizeof(double));
}

void Particles::attrdata::FromDisk(int i) {
	std::ifstream strm(diskFd + std::to_string(i), std::ios::binary);
	auto& d = dataAll[i];
	d.resize(Particles::particleSz);
	strm.read((char*)d.data(), d.size() * sizeof(double));
}

std::string Particles::attrdata::Export() {
	std::ostringstream strm;
	if ((!timed && !data.size())) {
		return "0";
	}
	strm << Particles::particleSz << " " << (timed? Particles::anim.frameCount : 1) << "\n";
	if (timed) {
		for (int a = 0; a < Particles::anim.frameCount; a++) {
			auto& d = Get(a);
			strm << d.size() << " ";
			for (auto& d2 : d) {
				strm << d2 << " ";
			}
		}
	}
	else {
		for (auto& d : data) {
			strm << d << " ";
		}
	}
	return strm.str();
}

void Particles::attrdata::Import(const std::string& buf) {
	std::istringstream strm(buf);
	int psz;
	strm >> psz;
	if (!psz) return;
	else if (psz != Particles::particleSz) {
		Debug::Warning("Attr::Import", "not atom count!");
		return;
	}
	int fsz;
	strm >> fsz;
	if (fsz != 1 && fsz != Particles::anim.frameCount) {
		Debug::Warning("Attr::Import", "not frame count!");
		return;
	}

	timed = (fsz > 1);
	if (timed) {
		int n;
		for (int f = 0; f < fsz; ++f) {
			strm >> n;
			if (!n) continue;
			auto& d = Get(f);
			d.resize(psz);
			for (auto& d2 : d) {
				strm >> d2;
			}
			if (f > 0) status[f-1] = FRAME_STATUS::WAITWRITE;
		}
	}
	else {
		data.resize(Particles::particleSz);
		for (auto& d : data) {
			strm >> d;
		}
	}
	dirty = true;
}