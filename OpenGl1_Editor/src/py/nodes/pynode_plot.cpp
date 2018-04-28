#include "pynode_plot.h"
#include "utils/plot.h"
#include "ui/icons.h"

PyNode_Plot::PyNode_Plot() : PyNode(nullptr) {
	width = 200;
	inputR.resize(1, nullptr);
	inputV.resize(1);
	inputV[0].first.type = PY_VARTYPE::LIST;
	inputV[0].first.typeName = "list(float)";
}

void PyNode_Plot::Draw() {
	auto cnt = 1;
	Engine::DrawQuad(pos.x, pos.y, width, 16, white(selected ? 1.0f : 0.7f, 0.35f));
	if (Engine::Button(pos.x, pos.y, 16, 16, expanded ? Icons::expand : Icons::collapse, white(0.8f), white(), white(0.5f)) == MOUSE_RELEASE) expanded = !expanded;
	UI::Label(pos.x + 20, pos.y + 2, 12, "Plot list(float)", font, white());
	if (expanded) {
		Engine::DrawQuad(pos.x, pos.y + 16, width, 3 + 17 * cnt + width, white(0.7f, 0.25f));
		float y = pos.y + 18;
		//for (uint i = 0; i < 1; i++, y += 17) {
		UI::Texture(pos.x - 5, y + 3, 10, 10, inputR[0] ? tex_circle_conn : tex_circle_open);
		UI::Label(pos.x + 10, y, 12, "value 1", font, white());
		//}
		if (valXs.size()) {
			plt::plot(pos.x + 2, pos.y + 18 + 17 * cnt, width - 4, width - 4, &valXs[0], &valYs[0], valXs.size());
		}
	}
}

void PyNode_Plot::DrawConn() {
	auto cnt = 1;
	float y = pos.y + 18;
	//for (uint i = 0; i < script->invarCnt; i++, y += 17) {
		if (inputR[0]) Engine::DrawLine(Vec2(pos.x, expanded ? y + 8 : pos.y + 8), Vec2(inputR[0]->pos.x + inputR[0]->width, inputR[0]->expanded ? inputR[0]->pos.y + 20 + 8 + (inputV[0].second + inputR[0]->script->invarCnt) * 17 : inputR[0]->pos.y + 8), white(), 2);
	//}
}

void PyNode_Plot::Execute() {
	if (!inputR[0]) return;
	auto ref = inputR[0]->outputV[inputV[0].second].first.value;
	auto sz = PyList_Size(ref);
	valXs.resize(sz);
	valYs.resize(sz);
	for (uint a = 0; a < sz; a++) {
		valXs[a] = (float)a;
		auto obj = PyList_GetItem(ref, (Py_ssize_t)a);
		valYs[a] = (float)PyFloat_AsDouble(obj);
	}
}