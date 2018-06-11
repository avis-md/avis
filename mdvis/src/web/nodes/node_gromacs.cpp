#include "../anweb.h"
#include "node_gromacs.h"
#ifndef IS_ANSERVER
#include "ui/icons.h"
#endif

Node_Gromacs::Node_Gromacs() : Node_Inputs() {
	script->name = ".ingro";
	title = "Gromacs File";
	titleCol = Vec3(0.225f, 0.5f, 0.25f);

	outputR.resize(4);
	script->outvars.resize(4);
	script->outvars[2].first = "residue IDs";
	script->outvars[3].first = "particle IDs";
	script->outvars[2].second = script->outvars[3].second = "list(1)";

	conV.resize(4);
	auto& ress = conV[2];
	ress.type = AN_VARTYPE::LIST;
	ress.dimVals.resize(1);
	conV[3] = ress;
}

void Node_Gromacs::Draw() {
#ifndef IS_ANSERVER
	auto cnt = 4;
	this->pos = pos;
	Engine::DrawQuad(pos.x, pos.y, width, 16, Vec4(titleCol, selected ? 1.0f : 0.7f));
	UI::Label(pos.x + 2, pos.y + 1, 12, title, font, white());
	Engine::DrawQuad(pos.x, pos.y + 16, width, 20 + 17 * cnt, white(0.7f, 0.25f));
	UI::Label(pos.x + 2, pos.y + 18, 12, "File", font, white());
	DrawToolbar();
	file = UI::EditText(pos.x + 32, pos.y + 18, width - 50, 16, 12, white(1, 0.5f), file, font, true, 0, white());
	if (Engine::Button(pos.x + width - 17, pos.y + 18, 16, 16, white(1, 0.5f)) == MOUSE_RELEASE) {
		file = IO::OpenFile("*.gro");
	}
	UI::Texture(pos.x + width - 17, pos.y + 18, 16, 16, Icons::browse);
	float y = pos.y + 37;
	for (uint i = 0; i < cnt; i++, y += 17) {
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

		font->Align(ALIGN_TOPRIGHT);
		UI::Label(pos.x + width - 10, y, 12, script->outvars[i].first, font, white());
		font->Align(ALIGN_TOPLEFT);
		UI::Label(pos.x + 2, y, 12, script->outvars[i].second, font, white(0.3f), width * 0.67f - 6);
	}
#endif
}

void Node_Gromacs::Execute() {
	
}