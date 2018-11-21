#include "Engine.h"

Mesh::Mesh() : vertCount(~0U) {}

Mesh::Mesh(int vsz, Vec3* pos, Vec3* norm, int tsz, int* tri, bool sv) : vertCount(vsz), triCount(tsz) {
	if (sv) {
		vertices.resize(vsz);
		normals.resize(vsz);
		memcpy(&vertices[0], pos, vsz * sizeof(Vec3));
		if (norm) memcpy(&normals[0], norm, vsz * sizeof(Vec3));
		triangles.resize(tsz * 3);
		memcpy(&triangles[0], tri, tsz * sizeof(int) * 3);
	}
	glGenVertexArrays(1, &vao);
	glGenBuffers(2, vbos);
	glBindBuffer(GL_ARRAY_BUFFER, vbos[0]); //pos
	glBufferData(GL_ARRAY_BUFFER, vsz * sizeof(Vec3), pos, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, vbos[1]); //norm
	glBufferData(GL_ARRAY_BUFFER, vsz * sizeof(Vec3), norm, GL_STATIC_DRAW);
	glBindVertexArray(vao);
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glBindBuffer(GL_ARRAY_BUFFER, vbos[0]);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glBindBuffer(GL_ARRAY_BUFFER, vbos[1]);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
	glGenBuffers(1, &veo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, veo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, tsz * 3 * sizeof(int), tri, GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

Mesh::~Mesh() {
	CheckUniqueRef();
}

void Mesh::DestroyRef() {
	if (vertCount != ~0U) {
		glDeleteBuffers(1, &veo);
		glDeleteBuffers(2, vbos);
		glDeleteVertexArrays(1, &vao);
	}
}