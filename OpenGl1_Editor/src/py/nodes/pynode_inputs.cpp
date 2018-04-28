#include "pynode_inputs.h"

PyNode_Inputs::PyNode_Inputs() : PyNode(nullptr) {
	width = 150;
	auto v = std::pair<PyVar, uint>();
	v.first.type = PY_VARTYPE::LIST;
	v.first.typeName = "list(float)";
	outputR.resize(2, nullptr);
	outputV.resize(2, v);
	outputV[0].first.name = "positions";
	outputV[1].first.name = "velocities";
}

void PyNode_Inputs::Draw() {
	auto cnt = 1;
	Engine::DrawQuad(pos.x, pos.y, width, 16, white(selected ? 1.0f : 0.7f, 0.35f));
	UI::Label(pos.x + 2, pos.y + 2, 12, "System", font, white());
	Engine::DrawQuad(pos.x, pos.y + 16, width, 3 + 17 * cnt + width, white(0.7f, 0.25f));
	float y = pos.y + 18;
	for (uint i = 0; i < 2; i++, y += 17) {
		UI::Texture(pos.x + width - 5, y + 3, 10, 10, outputR[i] ? tex_circle_conn : tex_circle_open);
		font->Align(ALIGN_TOPRIGHT);
		UI::Label(pos.x + width - 10, y, 12, outputV[i].first.name, font, white());
		font->Align(ALIGN_TOPLEFT);
		UI::Label(pos.x + 2, y, 12, outputV[i].first.typeName, font, white(0.3f), width * 0.67f - 6);
	}
}

void PyNode_Inputs::Execute() {
	
}