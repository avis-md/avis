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
	if (!frm || frm == ~0U || !timed) return data;
	else {
		if (!readonly && (status[frm - 1] == FRAME_STATUS::UNLOADED)) {
			FromDisk(frm - 1);
			status[frm - 1] = FRAME_STATUS::LOADED;
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
	status.resize(Particles::anim.frameCount - 1 + (int)readonly);
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

	if (dirty) {
		Seek(Particles::anim.currentFrame);
	}

	if (timed) {
		if (readonly) {
			for (uint f = 0; f < Particles::anim.frameCount; ++f) {
				if (status[f] == FRAME_STATUS::WAITWRITE) {
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
				case FRAME_STATUS::EMPTY:
					break;
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
	std::vector<FRAME_STATUS>().swap(status);
}

void Particles::attrdata::ToDisk(int i) {
	std::string path = diskFd + std::to_string(i);
	std::ofstream strm(path, std::ios::binary);
	if (!strm) {
		Debug::Error("Attr::FromDisk", "Failed to open " + path + "!");
		return;
	}
	auto& d = dataAll[i];
	strm.write((char*)d.data(), Particles::particleSz * sizeof(double));
}

void Particles::attrdata::FromDisk(int i) {
	std::string path = diskFd + std::to_string(i);
	std::ifstream strm(path, std::ios::binary);
	if (!strm) {
		Debug::Error("Attr::FromDisk", "Failed to open " + path + "!");
		return;
	}
	auto& d = dataAll[i];
	d.resize(Particles::particleSz);
	if (!strm.read((char*)d.data(), Particles::particleSz * sizeof(double))) {
		Debug::Error("Attr::FromDisk", "Failed to read " + path + " contents!");
		return;
	}
}

void Particles::attrdata::Export(std::ostream& strm) {
	if ((!timed && !data.size())) {
		strm.write("\0\0\0\0", 4);
		return;
	}
	strm.write((char*)&Particles::particleSz, 4);
	if (timed) {
		strm.write((char*)&Particles::anim.frameCount, 4);
		for (int a = 0; a < Particles::anim.frameCount; a++) {
			auto& d = Get(a);
			strm << d.size() << " ";
			for (auto& d2 : d) {
				strm << d2 << " ";
			}
			const auto sz = d.size();
			strm.write((char*)&sz, 4);
			strm.write((char*)d.data(), sz * sizeof(double));
		}
	}
	else {
		strm.write("\1\0\0\0", 4);
		strm.write((char*)data.data(), Particles::particleSz * sizeof(double));
	}
}

void Particles::attrdata::Import(std::istream& strm) {
	int psz = 0;
	strm.read((char*)&psz, 4);
	if (!psz) return;
	else if (psz != Particles::particleSz) {
		Debug::Warning("Attr::Import", "not atom count!");
		return;
	}
	int fsz = 0;
	strm.read((char*)&fsz, 4);
	if (fsz != 1 && fsz != Particles::anim.frameCount) {
		Debug::Warning("Attr::Import", "not frame count!");
		return;
	}

	timed = (fsz > 1);
	if (timed) {
		int n = 0;
		for (int f = 0; f < fsz; ++f) {
			strm.read((char*)&n, 4);
			if (!n) continue;
			auto& d = Get(f);
			d.resize(psz);
			strm.read((char*)d.data(), psz * sizeof(double));
			if (f > 0) status[f-1] = FRAME_STATUS::WAITWRITE;
		}
	}
	else {
		data.resize(psz);
		strm.read((char*)data.data(), psz * sizeof(double));
	}
	dirty = true;
}