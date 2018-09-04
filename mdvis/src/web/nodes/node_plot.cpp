#include "../anweb.h"
#ifndef IS_ANSERVER
#include "utils/plot.h"
#include "ui/icons.h"
#endif

Node_Plot::Node_Plot() : AnNode(new DmScript(sig)) {
	title = "Plot Y";
	canTile = true;
	inputR.resize(1);
	script->invars.push_back(std::pair<string, string>("array", "list(1*)"));
}

void Node_Plot::DrawFooter(float& y) {
	Engine::DrawQuad(pos.x, y, width, width, white(0.7f, 0.25f));
	if (valXs.size()) {
		plt::plot(pos.x + 12, y + 2, width - 14, width - 14, &valXs[0], &valYs[0], valXs.size(), UI::font, 10, white(1, 0.8f));
	}
	y += width;
}

void Node_Plot::Execute() {
#ifndef IS_ANSERVER
	if (!inputR[0].first) return;
	CVar& cv = inputR[0].first->conV[inputR[0].second];
	auto& sz = *cv.dimVals[0];
	valXs.resize(sz);
	valYs.resize(sz);
	for (int i = 0; i < sz; i++) {
		valXs[i] = (float)i;
	}
	switch (cv.typeName[6]) {
	case 's':
		for (int i = 0; i < sz; i++) {
			valYs[i] = (float)(*(short**)cv.value)[i];
		}
		break;
	case 'i':
		for (int i = 0; i < sz; i++) {
			valYs[i] = (float)(*(int**)cv.value)[i];
		}
		break;
	case 'd':
		for (int i = 0; i < sz; i++) {
			valYs[i] = (float)(*(double**)cv.value)[i];
		}
		break;
	default:
		Debug::Warning("Plot", "Unexpected data type " + cv.typeName + "!");
		valXs.clear();
		break;
	}
#endif
}

void Node_Plot::LoadOut(const string& path) {
	Execute();
}