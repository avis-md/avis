#include "../anweb.h"
#ifndef IS_ANSERVER
#include "utils/plot.h"
#include "ui/ui_ext.h"
#include "ui/icons.h"
#endif

Node_Plot::Node_Plot() : AnNode(new DmScript(sig)) {
	title = "Plot Data";
	canTile = true;
	inputR.resize(3, nodecon(0, 0, false));
	inputVDef.resize(3);
	script->invars.push_back(std::pair<std::string, std::string>("array", "list(**)"));
	script->invars.push_back(std::pair<std::string, std::string>("X ID", "int"));
	script->invars.push_back(std::pair<std::string, std::string>("Y ID", "int"));
	script->invaropts.resize(3);
}

void Node_Plot::DrawFooter(float& y) {
	UI::Quad(pos.x, y, width, width, bgCol);
	if (valXs.size()) {
		plt::plot(pos.x + 12, y + 2, width - 14, width - 14, &valXs[0], &_valYs[0], valXs.size(), _valYs.size(), UI::font, 10, white(1, 0.8f));
	}
	y += width;
}

void Node_Plot::Execute() {
#ifndef IS_ANSERVER
	if (!inputR[0].first) return;
	auto xid = inputR[1].first ? *(int*)inputR[1].first->conV[inputR[1].second].value : inputVDef[1].i;
	auto yid = inputR[2].first ? *(int*)inputR[2].first->conV[inputR[2].second].value : inputVDef[2].i;
	CVar& cv = inputR[0].first->conV[inputR[0].second];
	if (!cv.value) return;
	auto ds = cv.dimVals.size();
	auto sz = *cv.dimVals[0];
	auto sz2 = (cv.dimVals.size() > 1)? *cv.dimVals[1] : 1;
	if (sz > 1) {
		xid = Clamp(xid, -1, sz2 - 1);
		yid = Clamp(yid, -1, sz2 - 1);
	}
	valXs.resize(sz);
	valYs.resize(sz2);
	_valYs.resize(sz2);
	for (int a = 0; a < sz2; a++) {
		valYs[a].resize(sz);
		_valYs[a] = &valYs[a][0];
	}
	if (ds == 1 || xid == -1) {
		for (int i = 0; i < sz; i++) {
			valXs[i] = (float)i;
		}
	}
	else {
		switch (cv.typeName[6]) {
		case 's':
			for (int i = 0; i < sz; i++) {
				valXs[i] = (float)(*(short**)cv.value)[i * 2 + xid];
			}
			break;
		case 'i':
			for (int i = 0; i < sz; i++) {
				valXs[i] = (float)(*(int**)cv.value)[i * 2 + xid];
			}
			break;
		case 'd':
			for (int i = 0; i < sz; i++) {
				valXs[i] = (float)(*(double**)cv.value)[i * 2 + xid];
			}
			break;
		default:
			Debug::Warning("Plot", "Unexpected data type " + cv.typeName + "!");
			valXs.clear();
			break;
		}
	}
	if (ds == 2 && yid == -1) {
#define cs(_c, _t) case _c:\
			for (int i = 0; i < sz; i++) {\
				valYs[j][i] = (float)(*(_t**)cv.value)[i*sz2 + j];\
			} break
		for (int j = 0; j < sz2; j++) {
			switch (cv.typeName[6]) {
				cs('s', short);
				cs('i', int);
				cs('d', double);
			default:
				Debug::Warning("Plot", "Unexpected data type " + cv.typeName + "!");
				valXs.clear();
				return;
			}
		}
#undef cs
	}
	else {
#define cs(_c, _t) case _c:\
			for (int i = 0; i < sz; i++) {\
				valYs[0][i] = (float)(*(_t**)cv.value)[i*sz2 + j];\
			} break
		_valYs.resize(1);
		int j = (ds == 1) ? 0 : yid;
		switch (cv.typeName[6]) {
			cs('s', short);
			cs('i', int);
			cs('d', double);
		default:
			Debug::Warning("Plot", "Unexpected data type " + cv.typeName + "!");
			valXs.clear();
			return;
		}
	}
#endif
}

void Node_Plot::LoadOut(const std::string& path) {
	Execute();
}

void Node_Plot::OnConn(bool o, int i) {
	if (i == 0) {
		auto& cv = inputR[0].first->conV[inputR[0].second];
		auto sz = cv.dimVals.size();
		if (sz == 1) useids = false;
		else if (sz == 2) {
			auto v = *cv.dimVals[1];
			useids = (v > 1);
		}
		else {
			Debug::Warning("Node::Plot", "Data of 3+ dimensions cannot be plotted!");
		}
		inputR[1].use = inputR[2].use = useids;
	}
}

void Node_Plot::OnValChange(int i) {
	Execute();
}