#include "particles.h"
#include "utils/glext.h"

int Particles::attrdata::_ids = 0;

Particles::attrdata::attrdata() : instanceId(++_ids) {
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
	else return dataAll[frm-1];
}

void Particles::attrdata::ApplyParCnt() {
	if (!particleSz) return;
	SetGLBuf<float>(buf, nullptr, particleSz);
}

void Particles::attrdata::ApplyFrmCnt() {
	if (Particles::anim.frameCount < 2) return;
	dataAll.resize(Particles::anim.frameCount-1);
}

void Particles::attrdata::Update() {
	auto& dt = Get(anim.currentFrame);
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

}

void Particles::attrdata::FromDisk(int i) {

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