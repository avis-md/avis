#include "../anweb.h"
#ifndef IS_ANSERVER
#include "utils/plot.h"
#include "ui/icons.h"
#endif

Node_Plot::Node_Plot() : AnNode(new DmScript()) {
	//width = 200;
	canTile = true;
	inputR.resize(1);
	script->name = ".Plot";
	script->invars.push_back(std::pair<string, string>("array", "list(1)"));
}

void Node_Plot::Draw() {
#ifndef IS_ANSERVER
	auto cnt = 1;
	this->pos = pos;
	Engine::DrawQuad(pos.x, pos.y, width, 16, white(selected ? 1.0f : 0.7f, 0.35f));
	if (Engine::Button(pos.x, pos.y, 16, 16, expanded ? Icons::expand : Icons::collapse, white(0.8f), white(), white(0.5f)) == MOUSE_RELEASE) expanded = !expanded;
	UI::Label(pos.x + 20, pos.y + 1, 12, "Plot list(DIM=1)", font, white());
	DrawToolbar();
	if (expanded) {
		Engine::DrawQuad(pos.x, pos.y + 16, width, 3 + 17 * cnt + width, white(0.7f, 0.25f));
		float y = pos.y + 18;
		//for (uint i = 0; i < 1; i++, y += 17) {
		const uint i = 0;
		if (!AnWeb::selConnNode || (AnWeb::selConnIdIsOut && AnWeb::selConnNode != this)) {
			if (Engine::Button(pos.x - 5, y + 3, 10, 10, inputR[i].first ? tex_circle_conn : tex_circle_open, white(), white(), white()) == MOUSE_RELEASE) {
				if (!AnWeb::selConnNode) {
					AnWeb::selConnNode = this;
					AnWeb::selConnId = i;
					AnWeb::selConnIdIsOut = false;
					AnWeb::selPreClear = false;
				}
				else {
					AnWeb::selConnNode->ConnectTo(AnWeb::selConnId, this, i);
					AnWeb::selConnNode = nullptr;
				}
			}
		}
		else if (AnWeb::selConnNode == this && AnWeb::selConnId == i && !AnWeb::selConnIdIsOut) {
			Engine::DrawLine(Vec2(pos.x, y + 8), Input::mousePos, white(), 1);
			UI::Texture(pos.x - 5, y + 3, 10, 10, inputR[i].first ? tex_circle_conn : tex_circle_open);
		}
		else {
			UI::Texture(pos.x - 5, y + 3, 10, 10, inputR[i].first ? tex_circle_conn : tex_circle_open, red(0.3f));
		}
		//UI::Texture(pos.x - 5, y + 3, 10, 10, inputR[0].first ? tex_circle_conn : tex_circle_open);
		UI::Label(pos.x + 10, y, 12, "values", font, white());
		//}
		if (valXs.size()) {
			plt::plot(pos.x + 12, pos.y + 18 + 17 * cnt, width - 14, width - 14, &valXs[0], &valYs[0], valXs.size(), font, 10, white(1, 0.8f));
		}
	}
#endif
}

float Node_Plot::DrawSide() {
#ifndef IS_ANSERVER
	Engine::DrawQuad(pos.x, pos.y, width, 16, white(selected ? 1.0f : 0.7f, 0.35f));
	if (Engine::Button(pos.x, pos.y, 16, 16, expanded ? Icons::expand : Icons::collapse, white(0.8f), white(), white(0.5f)) == MOUSE_RELEASE) expanded = !expanded;
	UI::Label(pos.x + 20, pos.y + 1, 12, "Plot list(float)", font, white());
	if (expanded) {
		Engine::DrawQuad(pos.x, pos.y + 16, width, 4 + width, white(0.7f, 0.25f));
		if (valXs.size()) {
			plt::plot(pos.x + 12, pos.y + 18, width - 14, width - 14, &valXs[0], &valYs[0], valXs.size(), font, 10, white(1, 0.8f));
		}
		return width + 21;
	}
	else return 17;
#else
	return 0;
#endif
}

void Node_Plot::Execute() {
#ifndef IS_ANSERVER
	if (!inputR[0].first) return;
	CVar& cv = inputR[0].first->conV[inputR[0].second];
	auto& sz = *cv.dimVals[0];
	valXs.resize(sz);
	valYs.resize(sz);
	for (int i = 0; i < sz; i++) {
		valXs[i] = i;
	}
	float* src = *((float**)cv.value);
	memcpy(&valYs[0], src, sz * sizeof(float));
#endif
}

void Node_Plot::LoadOut(const string& path) {
	Execute();
}