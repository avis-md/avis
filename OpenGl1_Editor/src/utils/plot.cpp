#include "plot.h"

void plt::plot(float x, float y, float w, float h, float* dx, float* dy, uint cnt) {
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
}