#include "particles.h"
#include "parloader.h"
#include "web/anweb.h"

uint Particles::animdata::maxFramesInMem = 20;

void Particles::animdata::AllocFrames(uint frames) {
	frameCount = frames;
	status.resize(frames, FRAME_STATUS::UNLOADED);
	poss.resize(frames);
	vels.resize(frames);
	paths.resize(frames);
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
	poss.clear();
	vels.clear();
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
	if (frameCount <= 1) return;

	if (dirty) {
		dirty = false;
		for (auto& a : attrs) {
			a->ApplyFrmCnt();
		}
	}

	if (maxFramesInMem < 1000000) {
		if (maxFramesInMem < frameCount) {
			for (uint a = 0; a < frameMemPos; ++a) {
				if (a + frameCount - frameMemPos < maxFramesInMem) continue;
				if (status[a] == FRAME_STATUS::LOADED) {
					if (a == retainFrame) continue;
					std::vector<glm::dvec3>().swap(poss[a]);
					std::vector<glm::dvec3>().swap(vels[a]);
					status[a] = FRAME_STATUS::UNLOADED;
				}
			}
			for (uint a = frameMemPos + maxFramesInMem; a < frameCount; ++a) {
				if (status[a] == FRAME_STATUS::LOADED) {
					if (a == retainFrame) continue;
					std::vector<glm::dvec3>().swap(poss[a]);
					std::vector<glm::dvec3>().swap(vels[a]);
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
		skip:
			UpdateAttrs();
		}
	}
}

void Particles::animdata::UpdateMemRange() {
	frameMemPos = (uint)std::max((int)currentFrame - (int)maxFramesInMem/2, 0);
}