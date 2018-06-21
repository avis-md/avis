#include "plot.h"

string to_string_scientific(float f) {
	std::stringstream strm;
	strm.precision(3);
	strm << std::scientific << f;
	return strm.str();
}

void plt::plot(float x, float y, float w, float h, float* dx, float* dy, uint cnt, Font* font, uint sz, Vec4 col) {
	Engine::DrawQuad(x, y, w, h, black());
	x += 2;
	y += 2;
	w -= 4;
	h -= 4;
	Engine::DrawQuad(x, y, w, h, white());

	Vec3* poss = new Vec3[cnt];
	float x1 = *dx, x2 = *dx, y1 = *dy, y2 = *dy;
	for (uint i = 0; i < cnt; i++) {
		if (x1 > dx[i]) x1 = dx[i];
		if (x2 < dx[i]) x2 = dx[i];
		if (y1 > dy[i]) y1 = dy[i];
		if (y2 < dy[i]) y2 = dy[i];
	}
	for (uint i = 0; i < cnt; i++) {
		poss[i].x = ((x + ((dx[i] - x1) / (x2 - x1)) * w) / Display::width) * 2 - 1;
		poss[i].y = 1 - ((y + (1 - (dy[i] - y1) / (y2 - y1)) * h) / Display::height) * 2;
	}
	UI::SetVao(cnt, poss);

	glUseProgram(Engine::defProgram);
	glUniform4f(Engine::defColLoc, 0.0f, 0.0f, 0.0f, 1.0f);
	glBindVertexArray(UI::_vao);
	glDrawArrays(GL_LINE_STRIP, 0, cnt);
	glBindVertexArray(0);
	glUseProgram(0);

	delete[](poss);

	if (font) {
		font->Align(ALIGN_TOPLEFT);
		UI::Label(x, y + h + 2, (float)sz, to_string_scientific(x1), font, col);
		font->Align(ALIGN_TOPRIGHT);
		UI::Label(x + w, y + h + 2, (float)sz, to_string_scientific(x2), font, col);
		Engine::RotateUI(-90.0f, Vec2(x, y + w));
		font->Align(ALIGN_TOPLEFT);
		UI::Label(x, y + h - 4 - sz, (float)sz, to_string_scientific(y1), font, col);
		font->Align(ALIGN_TOPRIGHT);
		UI::Label(x + w, y + h - 4 - sz, (float)sz, to_string_scientific(y2), font, col);
		Engine::ResetUIMatrix();
		font->Align(ALIGN_TOPLEFT);
	}
}