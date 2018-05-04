#include "../PyWeb.h"

PyNode_Inputs::PyNode_Inputs() : PyNode(nullptr) {
	//width = 150;
	title = "All Particles";
	titleCol = Vec3(0.225f, 0.5f, 0.25f);
	canTile = true;
	auto v = std::pair<PyVar, uint>();
	v.first.type = PY_VARTYPE::LIST;
	v.first.typeName = "array(array(float))";
	outputR.resize(2, nullptr);
	outputV.resize(2, v);
	outputV[0].first.name = "positions";
	outputV[1].first.name = "velocities";
}

void PyNode_Inputs::Draw() {
	auto cnt = 2;
	this->pos = pos;
	Engine::DrawQuad(pos.x, pos.y, width, 16, Vec4(titleCol, selected ? 1.0f : 0.7f));
	UI::Label(pos.x + 2, pos.y + 1, 12, title, font, white());
	Engine::DrawQuad(pos.x, pos.y + 16, width, 3.0f + 17 * cnt, white(0.7f, 0.25f));
	float y = pos.y + 20;
	for (uint i = 0; i < 2; i++, y += 17) {
		if (!PyWeb::selConnNode || ((!PyWeb::selConnIdIsOut) && (PyWeb::selConnNode != this))) {
			if (Engine::Button(pos.x + width - 5, y + 3, 10, 10, outputR[i] ? tex_circle_conn : tex_circle_open, white(), white(), white()) == MOUSE_RELEASE) {
				if (!PyWeb::selConnNode) {
					PyWeb::selConnNode = this;
					PyWeb::selConnId = i;
					PyWeb::selConnIdIsOut = true;
					PyWeb::selPreClear = false;
				}
				else {
					ConnectTo(i, PyWeb::selConnNode, PyWeb::selConnId);
					PyWeb::selConnNode = nullptr;
				}
			}
		}
		else if (PyWeb::selConnNode == this && PyWeb::selConnId == i && PyWeb::selConnIdIsOut) {
			Engine::DrawLine(Vec2(pos.x + width, y + 8), Input::mousePos, white(), 1);
			UI::Texture(pos.x + width - 5, y + 3, 10, 10, outputR[i] ? tex_circle_conn : tex_circle_open);
		}
		else {
			UI::Texture(pos.x + width - 5, y + 3, 10, 10, outputR[i] ? tex_circle_conn : tex_circle_open, red(0.3f));
		}

		//UI::Texture(pos.x + width - 5, y + 3, 10, 10, outputR[i] ? tex_circle_conn : tex_circle_open);
		font->Align(ALIGN_TOPRIGHT);
		UI::Label(pos.x + width - 10, y, 12, outputV[i].first.name, font, white());
		font->Align(ALIGN_TOPLEFT);
		UI::Label(pos.x + 2, y, 12, outputV[i].first.typeName, font, white(0.3f), width * 0.67f - 6);
	}
}

Vec2 PyNode_Inputs::DrawConn() {
	return Vec2(width, 19 + 17 * 2);
}

void PyNode_Inputs::Execute() {
	
}


PyNode_Inputs_ActPar::PyNode_Inputs_ActPar() : PyNode_Inputs() {
	title = "Selected Particles";
	titleCol = Vec3(0.3f, 0.3f, 0.5f);
}