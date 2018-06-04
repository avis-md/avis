#include "../anweb.h"

Node_Inputs::Node_Inputs() : AnNode(new DmScript()) {
	DmScript* scr = (DmScript*)script;

	title = "All Particles";
	titleCol = Vec3(0.225f, 0.5f, 0.25f);
	canTile = true;
	auto v = std::pair<string, string>();
	v.second = "array(array(float))";
	outputR.resize(2);
	scr->outvars.resize(2, v);
	scr->outvars[0].first = "positions";
	scr->outvars[1].first = "velocities";
}

void Node_Inputs::Draw() {
	auto cnt = 2;
	this->pos = pos;
	Engine::DrawQuad(pos.x, pos.y, width, 16, Vec4(titleCol, selected ? 1.0f : 0.7f));
	UI::Label(pos.x + 2, pos.y + 1, 12, title, font, white());
	Engine::DrawQuad(pos.x, pos.y + 16, width, 3.0f + 17 * cnt, white(0.7f, 0.25f));
	float y = pos.y + 20;
	for (uint i = 0; i < 2; i++, y += 17) {
		if (!AnWeb::selConnNode || ((!AnWeb::selConnIdIsOut) && (AnWeb::selConnNode != this))) {
			if (Engine::Button(pos.x + width - 5, y + 3, 10, 10, outputR[i].first ? tex_circle_conn : tex_circle_open, white(), white(), white()) == MOUSE_RELEASE) {
				if (!AnWeb::selConnNode) {
					AnWeb::selConnNode = this;
					AnWeb::selConnId = i;
					AnWeb::selConnIdIsOut = true;
					AnWeb::selPreClear = false;
				}
				else {
					ConnectTo(i, AnWeb::selConnNode, AnWeb::selConnId);
					AnWeb::selConnNode = nullptr;
				}
			}
		}
		else if (AnWeb::selConnNode == this && AnWeb::selConnId == i && AnWeb::selConnIdIsOut) {
			Engine::DrawLine(Vec2(pos.x + width, y + 8), Input::mousePos, white(), 1);
			UI::Texture(pos.x + width - 5, y + 3, 10, 10, outputR[i].first ? tex_circle_conn : tex_circle_open);
		}
		else {
			UI::Texture(pos.x + width - 5, y + 3, 10, 10, outputR[i].first ? tex_circle_conn : tex_circle_open, red(0.3f));
		}

		//UI::Texture(pos.x + width - 5, y + 3, 10, 10, outputR[i] ? tex_circle_conn : tex_circle_open);
		font->Align(ALIGN_TOPRIGHT);
		UI::Label(pos.x + width - 10, y, 12, script->outvars[i].first, font, white());
		font->Align(ALIGN_TOPLEFT);
		UI::Label(pos.x + 2, y, 12, script->outvars[i].second, font, white(0.3f), width * 0.67f - 6);
	}
}

Vec2 Node_Inputs::DrawConn() {
	return Vec2(width, 19 + 17 * 2);
}

void Node_Inputs::Execute() {
	
}


Node_Inputs_ActPar::Node_Inputs_ActPar() : Node_Inputs() {
	title = "Selected Particles";
	titleCol = Vec3(0.3f, 0.3f, 0.5f);
}