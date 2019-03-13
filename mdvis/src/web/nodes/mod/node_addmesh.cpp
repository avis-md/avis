#include "node_addmesh.h"
#include "web/anweb.h"
#include "ui/ui_ext.h"
#include "vis/pargraphics.h"

Shader Node_AddMesh::shad;

INODE_DEF(__("Draw Mesh"), AddMesh, GEN);

Node_AddMesh::Node_AddMesh() : INODE_INIT, dirty(false), tsz(0), vao(0), vbos(), col(white()) {
	INODE_TITLE(NODE_COL_MOD)
	INODE_SINIT(
		scr->AddInput(_("vertices"), AN_VARTYPE::DOUBLE, 2);
		scr->AddInput(_("normals"), AN_VARTYPE::DOUBLE, 2);

		(shad = Shader::FromVF(IO::GetText(IO::path + "meshV.glsl"), IO::GetText(IO::path + "meshF.glsl")))
			.AddUniforms({ "_MV", "_P", "color" });
	);

	glGenVertexArrays(1, &vao);
	glGenBuffers(2, vbos);
	glBindVertexArray(vao);
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glBindBuffer(GL_ARRAY_BUFFER, vbos[0]);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glBindBuffer(GL_ARRAY_BUFFER, vbos[1]);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

Node_AddMesh::~Node_AddMesh() {
	glDeleteVertexArrays(1, &vao);
	glDeleteBuffers(2, vbos);
}

#define RETERR(msg) { std::cerr << msg << std::endl; return; }

void Node_AddMesh::Execute() {
	auto& ir0 = inputR[0];
	auto& ir1 = inputR[1];
	if (!ir0.first || !ir1.first) return;
	auto sz = ir0.getdim(0);
	if (sz != ir1.getdim(0)) RETERR("Vertex and normal array lengths are different!");
	if (ir0.getdim(1) != 3) RETERR("Dimension 1 of vertex array must be 3!");
	if (ir1.getdim(1) != 3) RETERR("Dimension 1 of normal array must be 3!");
	tsz = sz;
	poss.resize(sz * 3);
	nrms.resize(sz * 3);
	auto ps = *(double**)ir0.getval();
	auto nm = *(double**)ir1.getval();
	for (int a = 0; a < sz * 3; a++) {
		poss[a] = (float)ps[a];
		nrms[a] = (float)nm[a];
	}
	dirty = true;
}

void Node_AddMesh::Update() {
	if (dirty) {
		dirty = false;
		if (!tsz) return;
		glBindBuffer(GL_ARRAY_BUFFER, vbos[0]);
		glBufferData(GL_ARRAY_BUFFER, tsz * sizeof(Vec3), poss.data(), GL_STATIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, vbos[1]);
		glBufferData(GL_ARRAY_BUFFER, tsz * sizeof(Vec3), nrms.data(), GL_STATIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}
}

void Node_AddMesh::DrawHeader(float& off) {
	UI2::Color(pos.x + 2, off, width - 4, "Color", col);
	if (_col != col) {
		_col = col;
		Scene::dirty = true;
	}
	off += 17;
}

void Node_AddMesh::DrawScene(const RENDER_PASS pass) {
	if (!tsz) return;

	if (pass == RENDER_PASS::TRANS) {
		shad.Bind();
		glUniformMatrix4fv(shad.Loc(0), 1, GL_FALSE, glm::value_ptr(MVP::modelview()));
		glUniformMatrix4fv(shad.Loc(1), 1, GL_FALSE, glm::value_ptr(MVP::projection()));
		glBindVertexArray(vao);
		glUniform4f(shad.Loc(2), col.r, col.g, col.b, 0.f);
		glDrawArrays(GL_TRIANGLES, 0, tsz * 3);
		glUniform4f(shad.Loc(2), col.r, col.g, col.b, 0.8f);
		glDrawArrays(GL_TRIANGLES, 0, tsz * 3);
		glBindVertexArray(0);
		glUseProgram(0);
	}
}