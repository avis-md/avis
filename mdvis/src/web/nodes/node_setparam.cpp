#include "node_setparam.h"
#include "md/particles.h"
#include "ui/ui_ext.h"
#include "web/anweb.h"

Node_SetParam::Node_SetParam() : AnNode(new DmScript(sig)), paramId(0), di(&paramId, Particles::particles_ParamNms) {
	title = "Set Parameter";
	titleCol = NODE_COL_IO;
    canTile = false;
	inputR.resize(1);
	script->invars.push_back(std::pair<std::string, std::string>("values", "list(*d)"));
}

#define RETERR(msg) { std::cerr << msg << std::endl; return; }

void Node_SetParam::Execute() {
    if (!inputR[0].first) return;
	if (!Particles::particles_ParamSz) {
		RETERR("No params available!");
	}
	CVar& cv = inputR[0].first->conV[inputR[0].second];
	auto dm = cv.dimVals.size();
	auto src = *((double**)cv.value);
	auto& prm = Particles::particles_Params[paramId];
	if (dm > 2) {
		RETERR("Input must be 1- or 2-dimensional!");
	}
	else if (AnWeb::execFrame > 0 && dm > 1) {
		RETERR("Input must be 1-dimensional for Run All!");
	}
	auto sz = *cv.dimVals[0];
	auto tsz = 1;
	prm->timed = AnWeb::execFrame > 0;
	if (prm->timed) {
		if (sz != Particles::particleSz)
			RETERR("Input must be for each particle!");
		auto& tar = prm->data;
		tar.resize(Particles::anim.frameCount*sz);
#pragma omp parallel for
		for (int a = 0; a < sz; ++a)  {
			tar[a + Particles::particleSz*(AnWeb::execFrame-1)] = (float)src[a];
		}
	}
	else {
		if (dm == 2) {
			tsz = sz;
			sz = *cv.dimVals[1];
			if (tsz != Particles::anim.frameCount)
				RETERR("Input must be for each frame!");
			prm->timed = true;
		}

		if (sz != Particles::particleSz)
			RETERR("Input must be for each particle!");
		auto& tar = prm->data;
		tar.resize(tsz*sz);
#pragma omp parallel for
		for (int a = 0; a < sz*tsz; ++a)  {
			tar[a] = (float)src[a];
		}
	}
	prm->dirty = true;
}

void Node_SetParam::DrawHeader(float& off) {
	AnNode::DrawHeader(off);
	UI::Quad(pos.x, off, width, 18, bgCol);
	UI2::Dropdown(pos.x + 2, off, width - 4, "Parameter", di);
	off += 18;
}

void Node_SetParam::LoadOut(const std::string& path) {
	Execute();
}