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

#include "particles.h"
#include "imp/parloader.h"
#include "web/anweb.h"

uint Particles::animdata::maxFramesInMem = 20;

#define MAXFRAMES_INV 1000000

void Particles::animdata::AllocFrames(uint frames, bool c) {
	frameCount = frames;
	contiguous = c;
	status.resize(frames, FRAME_STATUS::UNLOADED);
	if (!c) {
		poss_s.resize(frames);
		vels_s.resize(frames);
		paths.resize(frames);
	}
}

void Particles::animdata::FillBBox() {
	bboxs.resize(frameCount);
	for (uint a = 0; a < frameCount; a++) {
		memcpy(&bboxs[a][0], boundingBox, 6*sizeof(double));
	}
	bboxState.resize(frameCount, BBOX_STATE::ORI);
}

void Particles::animdata::Clear() {
	frameCount = currentFrame = 0;
	status.clear();
	poss_s.clear();
	vels_s.clear();
	poss_a.clear();
	vels_a.clear();
	conns.clear();
	conns2.clear();
	bboxs.clear();
	paths.clear();
}

void Particles::animdata::Seek(uint f) {
	currentFrame = f;
	if (frameCount <= 1) return;
	if (status[f] != FRAME_STATUS::LOADED) {
		while (status[f] == FRAME_STATUS::READING){}
		if (status[f] == FRAME_STATUS::UNLOADED) {
			ParLoader::OpenFrameNow(f, paths[f]);
		}
		if (status[f] == FRAME_STATUS::BAD) return;
	}
	if (std::this_thread::get_id() == Engine::_mainThreadId) {
		for (auto& a : attrs) {
			a->Seek(currentFrame);
		}
	}
	UpdateMemRange();
}

void Particles::animdata::Update() {
	if (frameCount <= 1) goto skip;

	if (dirty) {
		dirty = false;
		for (auto& a : attrs) {
			a->ApplyFrmCnt();
		}
	}

	if (maxFramesInMem < MAXFRAMES_INV) {
		if (maxFramesInMem < frameCount) {
			for (uint a = 0; a < frameMemPos; ++a) {
				if (a + frameCount - frameMemPos < maxFramesInMem) continue;
				if (status[a] == FRAME_STATUS::LOADED) {
					if (a == retainFrame) continue;
					std::vector<glm::dvec3>().swap(poss_s[a]);
					std::vector<glm::dvec3>().swap(vels_s[a]);
					status[a] = FRAME_STATUS::UNLOADED;
				}
			}
			for (uint a = frameMemPos + maxFramesInMem; a < frameCount; ++a) {
				if (status[a] == FRAME_STATUS::LOADED) {
					if (a == retainFrame) continue;
					std::vector<glm::dvec3>().swap(poss_s[a]);
					std::vector<glm::dvec3>().swap(vels_s[a]);
					status[a] = FRAME_STATUS::UNLOADED;
				}
			}
		}

		if (!AnWeb::executing) {
			for (uint ff = currentFrame; ff < frameMemPos + maxFramesInMem; ++ff) {
				auto f = Repeat(ff, 0U, frameCount);
				auto& st = status[f];
				if (st == FRAME_STATUS::READING || st == FRAME_STATUS::BAD) return;
				if (st == FRAME_STATUS::UNLOADED) {
					st = FRAME_STATUS::READING;
					ParLoader::OpenFrame(f, paths[f]);
					goto skip;
				}
			}
			for (int f = currentFrame - 1; f >= (int)frameMemPos; --f) {
				auto& st = status[f];
				if (st == FRAME_STATUS::READING || st == FRAME_STATUS::BAD) return;
				if (st == FRAME_STATUS::UNLOADED) {
					st = FRAME_STATUS::READING;
					ParLoader::OpenFrame(f, paths[f]);
					break;
				}
			}
		}
	}
	skip:
		UpdateAttrs();
}

void Particles::animdata::UpdateMemRange() {
	frameMemPos = (uint)std::max((int)currentFrame - (int)maxFramesInMem/2, 0);
}

glm::dvec3* Particles::animdata::poss(int f) {
	if ((contiguous ? poss_a.empty() : poss_s.empty()))
		return nullptr;
	return contiguous ? &poss_a[f * particleSz] : &poss_s[f][0];
}

glm::dvec3* Particles::animdata::vels(int f) {
	if ((contiguous ? vels_a.empty() : vels_s.empty()))
		return nullptr;
	return contiguous ? &vels_a[f * particleSz] : &vels_s[f][0];
}

bool Particles::animdata::IsIncremental() {
	return maxFramesInMem < MAXFRAMES_INV;
}

void Particles::animdata::SetIncremental(bool b) {
	maxFramesInMem = b ? 20 : MAXFRAMES_INV;
}