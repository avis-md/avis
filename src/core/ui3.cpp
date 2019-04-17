#include "ui3.h"

GLuint UI3::cubeProgIds;

void UI3::Init() {
	const int cubeIds[] = {
		0,1,1,2,2,3,3,0,
		4,5,5,6,6,7,7,4,
		0,4,1,5,2,6,3,7
	};
	glGenBuffers(1, &cubeProgIds);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, cubeProgIds);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, 24 * sizeof(int), cubeIds, GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void UI3::Cube(Vec3 pos, float dx, float dy, float dz, Vec4 col) {
	Cube(pos.x-dx, pos.x+dx, pos.y-dy, pos.y+dy, pos.z-dz, pos.z+dz, col);
}

void UI3::Cube(float x1, float x2, float y1, float y2, float z1, float z2, Vec4 col) {
	Vec3 poss[8] = {
		Vec3(x1, y1, z1),
		Vec3(x2, y1, z1),
		Vec3(x2, y2, z1),
		Vec3(x1, y2, z1),
		Vec3(x1, y1, z2),
		Vec3(x2, y1, z2),
		Vec3(x2, y2, z2),
		Vec3(x1, y2, z2)
	};
	
	UI::SetVao(8, poss);
	auto mvp = MVP::projection() * MVP::modelview();

	Engine::defProgW.Bind();
	glUniform4f(Engine::defProgW.Loc(0), col.r, col.g, col.b, col.a);
	glUniformMatrix4fv(Engine::defProgW.Loc(1), 1, GL_FALSE, glm::value_ptr(mvp));
	glBindVertexArray(UI::_vao);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, cubeProgIds);
	glDrawElements(GL_LINES, 24, GL_UNSIGNED_INT, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
	glUseProgram(0);
}