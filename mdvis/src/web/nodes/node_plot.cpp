#include "../anweb.h"
#ifndef IS_ANSERVER
#include "utils/plot.h"
#include "ui/ui_ext.h"
#include "ui/icons.h"
#endif

Node_Plot::Node_Plot() : AnNode(new DmScript(sig)) {
	title = "Plot Data";
	canTile = true;
	inputR.resize(1);
	script->invars.push_back(std::pair<std::string, std::string>("array", "list(**)"));
}

void Node_Plot::DrawHeader(float& y) {
	if (inputR[0].first && useids) {
		UI::Quad(pos.x, y, width, 34, bgCol);
		auto xo = xid, yo = yid;
		xid = TryParse(UI2::EditText(pos.x + 2, y, (uint)width - 4U, "X axis index", std::to_string(xid)), 0);
		yid = TryParse(UI2::EditText(pos.x + 2, y + 17, (uint)width - 4U, "Y axis index", std::to_string(yid)), 0);
		auto s = *inputR[0].first->conV[inputR[0].second].dimVals[1];
		xid = Clamp(xid, -1, (int)s - 1);
		yid = Clamp(yid, -1, (int)s - 1);
		if (xid != xo || yid != yo) Execute();
		y += 34;
	}
}

void Node_Plot::DrawFooter(float& y) {
	UI::Quad(pos.x, y, width, width, bgCol);
	if (valXs.size()) {
		plt::plot(pos.x + 12, y + 2, width - 14, width - 14, &valXs[0], &valYs[0], valXs.size(), UI::font, 10, white(1, 0.8f));
	}
	y += width;
}

void Node_Plot::Execute() {
#ifndef IS_ANSERVER
	if (!inputR[0].first) return;
	CVar& cv = inputR[0].first->conV[inputR[0].second];
	auto ds = cv.dimVals.size();
	auto& sz = *cv.dimVals[0];
	valXs.resize(sz);
	valYs.resize(sz);
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
		for (int i = 0; i < sz; i++) {
			valYs[i] = (float)i;
		}
	}
	else {
		int j = (ds == 1) ? 0 : yid;
		switch (cv.typeName[6]) {
		case 's':
			for (int i = 0; i < sz; i++) {
				valYs[i] = (float)(*(short**)cv.value)[i*ds + j];
			}
			break;
		case 'i':
			for (int i = 0; i < sz; i++) {
				valYs[i] = (float)(*(int**)cv.value)[i*ds + j];
			}
			break;
		case 'd':
			for (int i = 0; i < sz; i++) {
				valYs[i] = (float)(*(double**)cv.value)[i*ds + j];
			}
			break;
		default:
			Debug::Warning("Plot", "Unexpected data type " + cv.typeName + "!");
			valXs.clear();
			break;
		}
	}
#endif
}

void Node_Plot::LoadOut(const std::string& path) {
	Execute();
}

void Node_Plot::OnConn(bool o, int i) {
	auto& cv = inputR[0].first->conV[inputR[0].second];
	auto sz = cv.dimVals.size();
	if (sz == 1) useids = false;
	else if (sz == 2) {
		auto v = *cv.dimVals[1];
		if (v > 1) {
			useids = true;
			if (xid >= v) xid = v - 1;
			if (yid >= v) yid = v - 1;
			if (xid == yid) xid--;
		}
		else useids = false;
	}
	else {
		Debug::Warning("Node::Plot", "Data of 3+ dimensions cannot be plotted!");
	}
}