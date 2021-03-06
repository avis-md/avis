// Copyright (C) 2019 Pua Kai
// 
// This file is part of AViS.
// 
// AViS is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// AViS is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with AViS.  If not, see <http://www.gnu.org/licenses/>.

#include "node_addsurface.h"
#include "vis/pargraphics.h"
#include "md/particles.h"
#include "ui/ui_ext.h"
#include "res/shd/marchVert.h"
#include "res/shd/marchGeom.h"
#include "res/shd/surfDraw.h"

INODE_DEF(__("Draw Surface"), AddSurface, GEN);

Shader Node_AddSurface::marchProg;
Shader Node_AddSurface::drawProg;

size_t Node_AddSurface::bufSz = 0;
int Node_AddSurface::maxBufSz = 0;
uint Node_AddSurface::genSz = 0;
GLuint Node_AddSurface::inBuf, Node_AddSurface::inBufT, Node_AddSurface::query;

std::mutex Node_AddSurface::lock;

Node_AddSurface::Node_AddSurface() : INODE_INITF(AN_FLAG_RUNONSEEK | AN_FLAG_RUNONVALCHG) {
	INODE_TITLE(NODE_COL_MOD)
	INODE_SINIT(
		Init();

		scr->AddInput(_("density"), AN_VARTYPE::DOUBLE, 3);
		scr->AddInput(_("value"), AN_VARTYPE::DOUBLE);

		scr->AddOutput(_("vertices"), AN_VARTYPE::DOUBLE, 2);
		scr->AddOutput(_("normals"), AN_VARTYPE::DOUBLE, 2);
	);

	IAddConV(0, { (int*)&genSz, nullptr }, { 3 });
	IAddConV(0, { (int*)&genSz, nullptr }, { 3 });

	InitBuffers();

	glGetIntegerv(GL_MAX_TEXTURE_BUFFER_SIZE, &maxBufSz);

	static std::string ss[] = {
		"Visualize",
		"Generate",
		""
	};

	mode = Mode::Draw;
	mode_di.target = (uint*)&mode;
	mode_di.list = &ss[0];
}

Node_AddSurface::~Node_AddSurface() {
	glDeleteVertexArrays(1, &vao);
	glDeleteBuffers(1, &outPos);
	glDeleteBuffers(1, &outNrm);
}

void Node_AddSurface::Update() {
	if (!!data.size()) {
		std::lock_guard<std::mutex> locker(lock);
		genSz = 0;
		const int mul = shape[1] * shape[2];
		const int zmax = maxBufSz / mul / sizeof(float);
		const int zn = (shape[0] + zmax - 1) / zmax;
		std::cout << "number of buffer splits: " << zn << " (" << zmax << " " << shape[2] << ")" << std::endl;

		int _zs = (shape[0] + zn * 2 - 1) / zn;
		ResizeInBuf(_zs * mul * sizeof(float));
		
		int zo = 0;
		int _outSz = 0;
		int _mtmpSz = 0;
		for (int a = 0; a < zn; a++) {
			auto zs = (a == zn - 1) ? std::min(_zs, shape[0] - zo) : _zs;
			Debug::Message("Node_AddSurface", "Generating part " + std::to_string(a + 1) + " of " + std::to_string(zn));
			std::cout << zo << " " << zs << std::endl;
			SetInBuf(&data[zo * mul], zs * mul * sizeof(float));
			auto sz = ExecMC(glm::ivec3(zo, 0, 0), glm::ivec3(zs, shape[1], shape[2]), glm::ivec3(shape[0], shape[1], shape[2]));
			_outSz += sz;
			_mtmpSz = std::max(_mtmpSz, sz);
			zo += zs - 1;
		}

		Debug::Message("Node_AddSurface", "Allocating for " + std::to_string(_outSz) + " triangles with tmp buffer of " + std::to_string(_mtmpSz) + " triangles");
		ResizeOutBuf(_outSz * 3 * sizeof(Vec3));
		ResizeTmpBuf(_mtmpSz * 3 * sizeof(Vec3));

		zo = 0;
		genSz = 0;
		for (int a = 0; a < zn; a++) {
			auto zs = (a == zn - 1) ? std::min(_zs, shape[0] - zo) : _zs;
			Debug::Message("Node_AddSurface", "Generating part " + std::to_string(a + 1) + " of " + std::to_string(zn));
			std::cout << zo << " " << zs << std::endl;
			SetInBuf(&data[zo * mul], zs * mul * sizeof(float));
			auto sz = ExecMC(glm::ivec3(zo, 0, 0), glm::ivec3(zs, shape[1], shape[2]), glm::ivec3(shape[0], shape[1], shape[2]));
			glBindBuffer(GL_COPY_READ_BUFFER, tmpPos);
			glBindBuffer(GL_COPY_WRITE_BUFFER, outPos);
			glCopyBufferSubData(GL_COPY_READ_BUFFER, GL_COPY_WRITE_BUFFER, 0, genSz * sizeof(Vec3), sz * 3 * sizeof(Vec3));
			glBindBuffer(GL_COPY_READ_BUFFER, tmpNrm);
			glBindBuffer(GL_COPY_WRITE_BUFFER, outNrm);
			glCopyBufferSubData(GL_COPY_READ_BUFFER, GL_COPY_WRITE_BUFFER, 0, genSz * sizeof(Vec3), sz * 3 * sizeof(Vec3));
			glBindBuffer(GL_COPY_READ_BUFFER, 0);
			glBindBuffer(GL_COPY_WRITE_BUFFER, 0);
			zo += zs - 1;
			genSz += sz * 3;
		}

		resultPos.resize(genSz * 3);
		resultNrm.resize(genSz * 3);

		glBindBuffer(GL_ARRAY_BUFFER, outPos);
		auto pos = (Vec3*)glMapBufferRange(GL_ARRAY_BUFFER, 0, genSz * sizeof(Vec3), GL_MAP_READ_BIT);
		std::memcpy(resultPos.data(), pos, genSz * sizeof(Vec3));
		glUnmapBuffer(GL_ARRAY_BUFFER);
		glBindBuffer(GL_ARRAY_BUFFER, outNrm);
		auto nrm = (Vec3*)glMapBufferRange(GL_ARRAY_BUFFER, 0, genSz * sizeof(Vec3), GL_MAP_READ_BIT);
		std::memcpy(resultNrm.data(), nrm, genSz * sizeof(Vec3));
		glUnmapBuffer(GL_ARRAY_BUFFER);
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		data.clear();
		Scene::dirty = true;
	}
}

void Node_AddSurface::DrawHeader(float& off) {
	AnNode::DrawHeader(off);

	UI2::Dropdown(pos.x + 5, off, width - 10, "Mode", mode_di);
	static auto _mode = mode;
	if (_mode != mode) {
		_mode = mode;
		
	}
	off += 17;

	UI2::Toggle(pos.x + 5, off, width - 10, "invert", invert);
	off += 17;
}

void Node_AddSurface::DrawScene(RENDER_PASS pass) {
	if (!genSz || (mode != Mode::Draw)) return;

	if (pass == RENDER_PASS::SOLID) {
		glUseProgram(drawProg);
		glUniformMatrix4fv(drawProg.Loc(0), 1, GL_FALSE, glm::value_ptr(MVP::modelview()));
		glUniformMatrix4fv(drawProg.Loc(1), 1, GL_FALSE, glm::value_ptr(MVP::projection() * MVP::modelview()));
		auto& bboxs = Particles::boundingBox;
		glUniform3f(drawProg.Loc(2), bboxs[0], bboxs[2], bboxs[4]);
		glUniform3f(drawProg.Loc(3), bboxs[1], bboxs[3], bboxs[5]);
		glUniform1f(drawProg.Loc(4), invert? -1.f : 1.f);
		glBindVertexArray(vao);
		glDrawArrays(GL_TRIANGLES, 0, genSz);
		glBindVertexArray(0);
		glUseProgram(0);
	}
}

void Node_AddSurface::RayTraceMesh(_Mesh& mesh) {
	if (!genSz) return;
	mesh.triCount = (mesh.vertCount = genSz) / 3;
	mesh.vertices.resize(mesh.vertCount);
	mesh.normals.resize(mesh.vertCount);
	mesh.triangles.resize(mesh.vertCount);
	for (uint a = 0; a < mesh.vertCount; a++) {
		mesh.vertices[a] = Vec3(
			Lerp(Particles::boundingBox[0], Particles::boundingBox[1], resultPos[a].x),
			Lerp(Particles::boundingBox[2], Particles::boundingBox[3], resultPos[a].y),
			Lerp(Particles::boundingBox[4], Particles::boundingBox[5], resultPos[a].z)
		);
		mesh.normals[a] = resultNrm[a];
		mesh.triangles[a] = a;
	}
}

void Node_AddSurface::Execute() {
	genSz = 0;
	auto& ir = inputR[0];
	if (!ir.first) return;
	const auto& vl = *(double**)ir.getval(ANVAR_ORDER::C);
	if (!vl) return;
	shape[0] = ir.getdim(0);
	shape[1] = ir.getdim(1);
	shape[2] = ir.getdim(2);
	bufSz = shape[0] * shape[1] * shape[2];
	if (!bufSz) return;
	if (bufSz > maxBufSz) {
		Debug::Warning("AddSurface", "Exceeded maximum allowed texel size! ("
			+ std::to_string(bufSz) + " required, " + std::to_string(maxBufSz) + " available)");
	}
	{
		std::lock_guard<std::mutex> locker(lock);
		data.resize(bufSz);
		for (int a = 0; a < bufSz; a++) {
			data[a] = (float)vl[a];
		}

		cutoff = getval_d(1);
	}
	
	//if (outputR[0].size() > 0) {
		while (!!data.size())
			Engine::Sleep(100);
		
		resultPosd.resize(genSz);
		resultNrmd.resize(genSz);
		for (int a = 0; a < genSz; a++) {
			resultPosd[a] = (glm::dvec3)resultPos[a];
			resultNrmd[a] = (glm::dvec3)resultNrm[a];
		}

		auto& ov = ((DmScript_I*)script.get())->outputVs;
		ov[0].val.p = resultPosd.data();
		ov[0].pval = &ov[0].val.p;
		ov[1].val.p = resultNrmd.data();
		ov[1].pval = &ov[1].val.p;
	//}
}

void Node_AddSurface::Init() {
	const GLuint _clipBindId = 11;
	GLuint vs, gs;
	std::string err;
	if (!Shader::LoadShader(GL_VERTEX_SHADER, glsl::marchVert, vs, &err)) {
		Debug::Error("AddSurface::Init", "Failed to load vertex shader! " + err);
		return;
	}
	if (!Shader::LoadShader(GL_GEOMETRY_SHADER, glsl::marchGeom, gs, &err)) {
		Debug::Error("AddSurface::Init", "Failed to load geometry shader! " + err);
		return;
	}

	marchProg = glCreateProgram();
	glAttachShader(marchProg, vs);
	glAttachShader(marchProg, gs);
	const char* fb[] = { "outPos", "outNrm" };
	glTransformFeedbackVaryings(marchProg, 2, fb, GL_SEPARATE_ATTRIBS);
	if (!Shader::LinkShader(marchProg)) {
		marchProg = 0;
		return;
	}

	glDetachShader(marchProg, vs);
	glDetachShader(marchProg, gs);
	glDeleteShader(vs);
	glDeleteShader(gs);

	marchProg.AddUniforms({ "data", "val", "shp", "off", "scl", "triBuf" });

	(drawProg = Shader(glsl::surfDVert, glsl::surfDFrag))
		.AddUniforms({ "_MV", "_MVP", "bbox1", "bbox2", "direction" });
	auto bid = glGetUniformBlockIndex(drawProg, "clipping");
	glUniformBlockBinding(drawProg, bid, _clipBindId);

	glGenBuffers(1, &inBuf);
	glGenTextures(1, &inBufT);
	glBindTexture(GL_TEXTURE_BUFFER, inBufT);
	glTexBuffer(GL_TEXTURE_BUFFER, GL_R32F, inBuf);
	glBindTexture(GL_TEXTURE_BUFFER, 0);
	glGenQueries(1, &query);

	glGenBuffers(1, &triBuf);
	glGenTextures(1, &triBufT);
	glBindBuffer(GL_ARRAY_BUFFER, triBuf);
	glBufferData(GL_ARRAY_BUFFER, sizeof(triTable), triTable, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindTexture(GL_TEXTURE_BUFFER, triBufT);
	glTexBuffer(GL_TEXTURE_BUFFER, GL_R32I, triBuf);
	glBindTexture(GL_TEXTURE_BUFFER, 0);
}

void Node_AddSurface::InitBuffers() {
	glGenVertexArrays(1, &vao);
	glGenBuffers(1, &outPos);
	glGenBuffers(1, &outNrm);
	glGenBuffers(1, &tmpPos);
	glGenBuffers(1, &tmpNrm);

	glBindVertexArray(vao);
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glBindBuffer(GL_ARRAY_BUFFER, outPos);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glBindBuffer(GL_ARRAY_BUFFER, outNrm);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glBindVertexArray(0);
}

void Node_AddSurface::ResizeInBuf(int i) {
	glBindBuffer(GL_ARRAY_BUFFER, inBuf);
	glBufferData(GL_ARRAY_BUFFER, i, nullptr, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindTexture(GL_TEXTURE_BUFFER, inBufT);
	glTexBuffer(GL_TEXTURE_BUFFER, GL_R32F, inBuf);
	glBindTexture(GL_TEXTURE_BUFFER, 0);
}

void Node_AddSurface::ResizeOutBuf(int i) {
	glBindBuffer(GL_ARRAY_BUFFER, outPos);
	glBufferData(GL_ARRAY_BUFFER, i, nullptr, GL_STATIC_COPY);
	glBindBuffer(GL_ARRAY_BUFFER, outNrm);
	glBufferData(GL_ARRAY_BUFFER, i, nullptr, GL_STATIC_COPY);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void Node_AddSurface::ResizeTmpBuf(int i) {
	glBindBuffer(GL_ARRAY_BUFFER, tmpPos);
	glBufferData(GL_ARRAY_BUFFER, i, nullptr, GL_STATIC_COPY);
	glBindBuffer(GL_ARRAY_BUFFER, tmpNrm);
	glBufferData(GL_ARRAY_BUFFER, i, nullptr, GL_STATIC_COPY);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void Node_AddSurface::SetInBuf(void* data, int sz) {
	glBindBuffer(GL_ARRAY_BUFFER, inBuf);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sz, data);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

int Node_AddSurface::ExecMC(glm::ivec3 offset, glm::ivec3 size, glm::ivec3 rsize) {
	glUseProgram(marchProg);
	glBindVertexArray(Camera::emptyVao);
	glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, tmpPos);
	glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 1, tmpNrm);
	glUniform1i(marchProg.Loc(0), 1);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_BUFFER, inBufT);
	glUniform1f(marchProg.Loc(1), cutoff);
	glUniform3i(marchProg.Loc(2), size[0], size[1], size[2]);
	glUniform3f(marchProg.Loc(3), offset[0] * 1.f / rsize[0], offset[1] * 1.f / rsize[1], offset[2] * 1.f / rsize[2]);
	glUniform3f(marchProg.Loc(4), size[0] * 1.f / rsize[0], size[1] * 1.f / rsize[1], size[2] * 1.f / rsize[2]);
	glUniform1i(marchProg.Loc(5), 2);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_BUFFER, triBufT);

	glEnable(GL_RASTERIZER_DISCARD);
	glBeginQuery(GL_PRIMITIVES_GENERATED, query);
	glBeginTransformFeedback(GL_TRIANGLES);
	const auto nv = (size[0]-1)*(size[1]-1)*(size[2]-1);
	glDrawArrays(GL_POINTS, 0, nv);
	glEndTransformFeedback();
	glEndQuery(GL_PRIMITIVES_GENERATED);
	uint gsz;
	glGetQueryObjectuiv(query, GL_QUERY_RESULT, &gsz);
	Debug::Message("Node_AddSurface", "Generated " + std::to_string(gsz) + " triangles");
	glDisable(GL_RASTERIZER_DISCARD);
	glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, 0);
	glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 1, 0);
	glBindVertexArray(0);
	glUseProgram(0);
	return (int)gsz;
}

const int Node_AddSurface::triTable[] = {
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	0, 8, 3, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	0, 1, 9, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	1, 8, 3, 9, 8, 1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	1, 2, 10, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	0, 8, 3, 1, 2, 10, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	9, 2, 10, 0, 2, 9, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	2, 8, 3, 2, 10, 8, 10, 9, 8, -1, -1, -1, -1, -1, -1,
	3, 11, 2, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	0, 11, 2, 8, 11, 0, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	1, 9, 0, 2, 3, 11, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	1, 11, 2, 1, 9, 11, 9, 8, 11, -1, -1, -1, -1, -1, -1,
	3, 10, 1, 11, 10, 3, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	0, 10, 1, 0, 8, 10, 8, 11, 10, -1, -1, -1, -1, -1, -1,
	3, 9, 0, 3, 11, 9, 11, 10, 9, -1, -1, -1, -1, -1, -1,
	9, 8, 10, 10, 8, 11, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	4, 7, 8, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	4, 3, 0, 7, 3, 4, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	0, 1, 9, 8, 4, 7, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	4, 1, 9, 4, 7, 1, 7, 3, 1, -1, -1, -1, -1, -1, -1,
	1, 2, 10, 8, 4, 7, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	3, 4, 7, 3, 0, 4, 1, 2, 10, -1, -1, -1, -1, -1, -1,
	9, 2, 10, 9, 0, 2, 8, 4, 7, -1, -1, -1, -1, -1, -1,
	2, 10, 9, 2, 9, 7, 2, 7, 3, 7, 9, 4, -1, -1, -1,
	8, 4, 7, 3, 11, 2, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	11, 4, 7, 11, 2, 4, 2, 0, 4, -1, -1, -1, -1, -1, -1,
	9, 0, 1, 8, 4, 7, 2, 3, 11, -1, -1, -1, -1, -1, -1,
	4, 7, 11, 9, 4, 11, 9, 11, 2, 9, 2, 1, -1, -1, -1,
	3, 10, 1, 3, 11, 10, 7, 8, 4, -1, -1, -1, -1, -1, -1,
	1, 11, 10, 1, 4, 11, 1, 0, 4, 7, 11, 4, -1, -1, -1,
	4, 7, 8, 9, 0, 11, 9, 11, 10, 11, 0, 3, -1, -1, -1,
	4, 7, 11, 4, 11, 9, 9, 11, 10, -1, -1, -1, -1, -1, -1,
	9, 5, 4, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	9, 5, 4, 0, 8, 3, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	0, 5, 4, 1, 5, 0, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	8, 5, 4, 8, 3, 5, 3, 1, 5, -1, -1, -1, -1, -1, -1,
	1, 2, 10, 9, 5, 4, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	3, 0, 8, 1, 2, 10, 4, 9, 5, -1, -1, -1, -1, -1, -1,
	5, 2, 10, 5, 4, 2, 4, 0, 2, -1, -1, -1, -1, -1, -1,
	2, 10, 5, 3, 2, 5, 3, 5, 4, 3, 4, 8, -1, -1, -1,
	9, 5, 4, 2, 3, 11, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	0, 11, 2, 0, 8, 11, 4, 9, 5, -1, -1, -1, -1, -1, -1,
	0, 5, 4, 0, 1, 5, 2, 3, 11, -1, -1, -1, -1, -1, -1,
	2, 1, 5, 2, 5, 8, 2, 8, 11, 4, 8, 5, -1, -1, -1,
	10, 3, 11, 10, 1, 3, 9, 5, 4, -1, -1, -1, -1, -1, -1,
	4, 9, 5, 0, 8, 1, 8, 10, 1, 8, 11, 10, -1, -1, -1,
	5, 4, 0, 5, 0, 11, 5, 11, 10, 11, 0, 3, -1, -1, -1,
	5, 4, 8, 5, 8, 10, 10, 8, 11, -1, -1, -1, -1, -1, -1,
	9, 7, 8, 5, 7, 9, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	9, 3, 0, 9, 5, 3, 5, 7, 3, -1, -1, -1, -1, -1, -1,
	0, 7, 8, 0, 1, 7, 1, 5, 7, -1, -1, -1, -1, -1, -1,
	1, 5, 3, 3, 5, 7, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	9, 7, 8, 9, 5, 7, 10, 1, 2, -1, -1, -1, -1, -1, -1,
	10, 1, 2, 9, 5, 0, 5, 3, 0, 5, 7, 3, -1, -1, -1,
	8, 0, 2, 8, 2, 5, 8, 5, 7, 10, 5, 2, -1, -1, -1,
	2, 10, 5, 2, 5, 3, 3, 5, 7, -1, -1, -1, -1, -1, -1,
	7, 9, 5, 7, 8, 9, 3, 11, 2, -1, -1, -1, -1, -1, -1,
	9, 5, 7, 9, 7, 2, 9, 2, 0, 2, 7, 11, -1, -1, -1,
	2, 3, 11, 0, 1, 8, 1, 7, 8, 1, 5, 7, -1, -1, -1,
	11, 2, 1, 11, 1, 7, 7, 1, 5, -1, -1, -1, -1, -1, -1,
	9, 5, 8, 8, 5, 7, 10, 1, 3, 10, 3, 11, -1, -1, -1,
	5, 7, 0, 5, 0, 9, 7, 11, 0, 1, 0, 10, 11, 10, 0,
	11, 10, 0, 11, 0, 3, 10, 5, 0, 8, 0, 7, 5, 7, 0,
	11, 10, 5, 7, 11, 5, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	10, 6, 5, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	0, 8, 3, 5, 10, 6, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	9, 0, 1, 5, 10, 6, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	1, 8, 3, 1, 9, 8, 5, 10, 6, -1, -1, -1, -1, -1, -1,
	1, 6, 5, 2, 6, 1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	1, 6, 5, 1, 2, 6, 3, 0, 8, -1, -1, -1, -1, -1, -1,
	9, 6, 5, 9, 0, 6, 0, 2, 6, -1, -1, -1, -1, -1, -1,
	5, 9, 8, 5, 8, 2, 5, 2, 6, 3, 2, 8, -1, -1, -1,
	2, 3, 11, 10, 6, 5, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	11, 0, 8, 11, 2, 0, 10, 6, 5, -1, -1, -1, -1, -1, -1,
	0, 1, 9, 2, 3, 11, 5, 10, 6, -1, -1, -1, -1, -1, -1,
	5, 10, 6, 1, 9, 2, 9, 11, 2, 9, 8, 11, -1, -1, -1,
	6, 3, 11, 6, 5, 3, 5, 1, 3, -1, -1, -1, -1, -1, -1,
	0, 8, 11, 0, 11, 5, 0, 5, 1, 5, 11, 6, -1, -1, -1,
	3, 11, 6, 0, 3, 6, 0, 6, 5, 0, 5, 9, -1, -1, -1,
	6, 5, 9, 6, 9, 11, 11, 9, 8, -1, -1, -1, -1, -1, -1,
	5, 10, 6, 4, 7, 8, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	4, 3, 0, 4, 7, 3, 6, 5, 10, -1, -1, -1, -1, -1, -1,
	1, 9, 0, 5, 10, 6, 8, 4, 7, -1, -1, -1, -1, -1, -1,
	10, 6, 5, 1, 9, 7, 1, 7, 3, 7, 9, 4, -1, -1, -1,
	6, 1, 2, 6, 5, 1, 4, 7, 8, -1, -1, -1, -1, -1, -1,
	1, 2, 5, 5, 2, 6, 3, 0, 4, 3, 4, 7, -1, -1, -1,
	8, 4, 7, 9, 0, 5, 0, 6, 5, 0, 2, 6, -1, -1, -1,
	7, 3, 9, 7, 9, 4, 3, 2, 9, 5, 9, 6, 2, 6, 9,
	3, 11, 2, 7, 8, 4, 10, 6, 5, -1, -1, -1, -1, -1, -1,
	5, 10, 6, 4, 7, 2, 4, 2, 0, 2, 7, 11, -1, -1, -1,
	0, 1, 9, 4, 7, 8, 2, 3, 11, 5, 10, 6, -1, -1, -1,
	9, 2, 1, 9, 11, 2, 9, 4, 11, 7, 11, 4, 5, 10, 6,
	8, 4, 7, 3, 11, 5, 3, 5, 1, 5, 11, 6, -1, -1, -1,
	5, 1, 11, 5, 11, 6, 1, 0, 11, 7, 11, 4, 0, 4, 11,
	0, 5, 9, 0, 6, 5, 0, 3, 6, 11, 6, 3, 8, 4, 7,
	6, 5, 9, 6, 9, 11, 4, 7, 9, 7, 11, 9, -1, -1, -1,
	10, 4, 9, 6, 4, 10, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	4, 10, 6, 4, 9, 10, 0, 8, 3, -1, -1, -1, -1, -1, -1,
	10, 0, 1, 10, 6, 0, 6, 4, 0, -1, -1, -1, -1, -1, -1,
	8, 3, 1, 8, 1, 6, 8, 6, 4, 6, 1, 10, -1, -1, -1,
	1, 4, 9, 1, 2, 4, 2, 6, 4, -1, -1, -1, -1, -1, -1,
	3, 0, 8, 1, 2, 9, 2, 4, 9, 2, 6, 4, -1, -1, -1,
	0, 2, 4, 4, 2, 6, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	8, 3, 2, 8, 2, 4, 4, 2, 6, -1, -1, -1, -1, -1, -1,
	10, 4, 9, 10, 6, 4, 11, 2, 3, -1, -1, -1, -1, -1, -1,
	0, 8, 2, 2, 8, 11, 4, 9, 10, 4, 10, 6, -1, -1, -1,
	3, 11, 2, 0, 1, 6, 0, 6, 4, 6, 1, 10, -1, -1, -1,
	6, 4, 1, 6, 1, 10, 4, 8, 1, 2, 1, 11, 8, 11, 1,
	9, 6, 4, 9, 3, 6, 9, 1, 3, 11, 6, 3, -1, -1, -1,
	8, 11, 1, 8, 1, 0, 11, 6, 1, 9, 1, 4, 6, 4, 1,
	3, 11, 6, 3, 6, 0, 0, 6, 4, -1, -1, -1, -1, -1, -1,
	6, 4, 8, 11, 6, 8, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	7, 10, 6, 7, 8, 10, 8, 9, 10, -1, -1, -1, -1, -1, -1,
	0, 7, 3, 0, 10, 7, 0, 9, 10, 6, 7, 10, -1, -1, -1,
	10, 6, 7, 1, 10, 7, 1, 7, 8, 1, 8, 0, -1, -1, -1,
	10, 6, 7, 10, 7, 1, 1, 7, 3, -1, -1, -1, -1, -1, -1,
	1, 2, 6, 1, 6, 8, 1, 8, 9, 8, 6, 7, -1, -1, -1,
	2, 6, 9, 2, 9, 1, 6, 7, 9, 0, 9, 3, 7, 3, 9,
	7, 8, 0, 7, 0, 6, 6, 0, 2, -1, -1, -1, -1, -1, -1,
	7, 3, 2, 6, 7, 2, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	2, 3, 11, 10, 6, 8, 10, 8, 9, 8, 6, 7, -1, -1, -1,
	2, 0, 7, 2, 7, 11, 0, 9, 7, 6, 7, 10, 9, 10, 7,
	1, 8, 0, 1, 7, 8, 1, 10, 7, 6, 7, 10, 2, 3, 11,
	11, 2, 1, 11, 1, 7, 10, 6, 1, 6, 7, 1, -1, -1, -1,
	8, 9, 6, 8, 6, 7, 9, 1, 6, 11, 6, 3, 1, 3, 6,
	0, 9, 1, 11, 6, 7, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	7, 8, 0, 7, 0, 6, 3, 11, 0, 11, 6, 0, -1, -1, -1,
	7, 11, 6, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	7, 6, 11, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	3, 0, 8, 11, 7, 6, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	0, 1, 9, 11, 7, 6, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	8, 1, 9, 8, 3, 1, 11, 7, 6, -1, -1, -1, -1, -1, -1,
	10, 1, 2, 6, 11, 7, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	1, 2, 10, 3, 0, 8, 6, 11, 7, -1, -1, -1, -1, -1, -1,
	2, 9, 0, 2, 10, 9, 6, 11, 7, -1, -1, -1, -1, -1, -1,
	6, 11, 7, 2, 10, 3, 10, 8, 3, 10, 9, 8, -1, -1, -1,
	7, 2, 3, 6, 2, 7, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	7, 0, 8, 7, 6, 0, 6, 2, 0, -1, -1, -1, -1, -1, -1,
	2, 7, 6, 2, 3, 7, 0, 1, 9, -1, -1, -1, -1, -1, -1,
	1, 6, 2, 1, 8, 6, 1, 9, 8, 8, 7, 6, -1, -1, -1,
	10, 7, 6, 10, 1, 7, 1, 3, 7, -1, -1, -1, -1, -1, -1,
	10, 7, 6, 1, 7, 10, 1, 8, 7, 1, 0, 8, -1, -1, -1,
	0, 3, 7, 0, 7, 10, 0, 10, 9, 6, 10, 7, -1, -1, -1,
	7, 6, 10, 7, 10, 8, 8, 10, 9, -1, -1, -1, -1, -1, -1,
	6, 8, 4, 11, 8, 6, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	3, 6, 11, 3, 0, 6, 0, 4, 6, -1, -1, -1, -1, -1, -1,
	8, 6, 11, 8, 4, 6, 9, 0, 1, -1, -1, -1, -1, -1, -1,
	9, 4, 6, 9, 6, 3, 9, 3, 1, 11, 3, 6, -1, -1, -1,
	6, 8, 4, 6, 11, 8, 2, 10, 1, -1, -1, -1, -1, -1, -1,
	1, 2, 10, 3, 0, 11, 0, 6, 11, 0, 4, 6, -1, -1, -1,
	4, 11, 8, 4, 6, 11, 0, 2, 9, 2, 10, 9, -1, -1, -1,
	10, 9, 3, 10, 3, 2, 9, 4, 3, 11, 3, 6, 4, 6, 3,
	8, 2, 3, 8, 4, 2, 4, 6, 2, -1, -1, -1, -1, -1, -1,
	0, 4, 2, 4, 6, 2, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	1, 9, 0, 2, 3, 4, 2, 4, 6, 4, 3, 8, -1, -1, -1,
	1, 9, 4, 1, 4, 2, 2, 4, 6, -1, -1, -1, -1, -1, -1,
	8, 1, 3, 8, 6, 1, 8, 4, 6, 6, 10, 1, -1, -1, -1,
	10, 1, 0, 10, 0, 6, 6, 0, 4, -1, -1, -1, -1, -1, -1,
	4, 6, 3, 4, 3, 8, 6, 10, 3, 0, 3, 9, 10, 9, 3,
	10, 9, 4, 6, 10, 4, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	4, 9, 5, 7, 6, 11, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	0, 8, 3, 4, 9, 5, 11, 7, 6, -1, -1, -1, -1, -1, -1,
	5, 0, 1, 5, 4, 0, 7, 6, 11, -1, -1, -1, -1, -1, -1,
	11, 7, 6, 8, 3, 4, 3, 5, 4, 3, 1, 5, -1, -1, -1,
	9, 5, 4, 10, 1, 2, 7, 6, 11, -1, -1, -1, -1, -1, -1,
	6, 11, 7, 1, 2, 10, 0, 8, 3, 4, 9, 5, -1, -1, -1,
	7, 6, 11, 5, 4, 10, 4, 2, 10, 4, 0, 2, -1, -1, -1,
	3, 4, 8, 3, 5, 4, 3, 2, 5, 10, 5, 2, 11, 7, 6,
	7, 2, 3, 7, 6, 2, 5, 4, 9, -1, -1, -1, -1, -1, -1,
	9, 5, 4, 0, 8, 6, 0, 6, 2, 6, 8, 7, -1, -1, -1,
	3, 6, 2, 3, 7, 6, 1, 5, 0, 5, 4, 0, -1, -1, -1,
	6, 2, 8, 6, 8, 7, 2, 1, 8, 4, 8, 5, 1, 5, 8,
	9, 5, 4, 10, 1, 6, 1, 7, 6, 1, 3, 7, -1, -1, -1,
	1, 6, 10, 1, 7, 6, 1, 0, 7, 8, 7, 0, 9, 5, 4,
	4, 0, 10, 4, 10, 5, 0, 3, 10, 6, 10, 7, 3, 7, 10,
	7, 6, 10, 7, 10, 8, 5, 4, 10, 4, 8, 10, -1, -1, -1,
	6, 9, 5, 6, 11, 9, 11, 8, 9, -1, -1, -1, -1, -1, -1,
	3, 6, 11, 0, 6, 3, 0, 5, 6, 0, 9, 5, -1, -1, -1,
	0, 11, 8, 0, 5, 11, 0, 1, 5, 5, 6, 11, -1, -1, -1,
	6, 11, 3, 6, 3, 5, 5, 3, 1, -1, -1, -1, -1, -1, -1,
	1, 2, 10, 9, 5, 11, 9, 11, 8, 11, 5, 6, -1, -1, -1,
	0, 11, 3, 0, 6, 11, 0, 9, 6, 5, 6, 9, 1, 2, 10,
	11, 8, 5, 11, 5, 6, 8, 0, 5, 10, 5, 2, 0, 2, 5,
	6, 11, 3, 6, 3, 5, 2, 10, 3, 10, 5, 3, -1, -1, -1,
	5, 8, 9, 5, 2, 8, 5, 6, 2, 3, 8, 2, -1, -1, -1,
	9, 5, 6, 9, 6, 0, 0, 6, 2, -1, -1, -1, -1, -1, -1,
	1, 5, 8, 1, 8, 0, 5, 6, 8, 3, 8, 2, 6, 2, 8,
	1, 5, 6, 2, 1, 6, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	1, 3, 6, 1, 6, 10, 3, 8, 6, 5, 6, 9, 8, 9, 6,
	10, 1, 0, 10, 0, 6, 9, 5, 0, 5, 6, 0, -1, -1, -1,
	0, 3, 8, 5, 6, 10, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	10, 5, 6, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	11, 5, 10, 7, 5, 11, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	11, 5, 10, 11, 7, 5, 8, 3, 0, -1, -1, -1, -1, -1, -1,
	5, 11, 7, 5, 10, 11, 1, 9, 0, -1, -1, -1, -1, -1, -1,
	10, 7, 5, 10, 11, 7, 9, 8, 1, 8, 3, 1, -1, -1, -1,
	11, 1, 2, 11, 7, 1, 7, 5, 1, -1, -1, -1, -1, -1, -1,
	0, 8, 3, 1, 2, 7, 1, 7, 5, 7, 2, 11, -1, -1, -1,
	9, 7, 5, 9, 2, 7, 9, 0, 2, 2, 11, 7, -1, -1, -1,
	7, 5, 2, 7, 2, 11, 5, 9, 2, 3, 2, 8, 9, 8, 2,
	2, 5, 10, 2, 3, 5, 3, 7, 5, -1, -1, -1, -1, -1, -1,
	8, 2, 0, 8, 5, 2, 8, 7, 5, 10, 2, 5, -1, -1, -1,
	9, 0, 1, 5, 10, 3, 5, 3, 7, 3, 10, 2, -1, -1, -1,
	9, 8, 2, 9, 2, 1, 8, 7, 2, 10, 2, 5, 7, 5, 2,
	1, 3, 5, 3, 7, 5, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	0, 8, 7, 0, 7, 1, 1, 7, 5, -1, -1, -1, -1, -1, -1,
	9, 0, 3, 9, 3, 5, 5, 3, 7, -1, -1, -1, -1, -1, -1,
	9, 8, 7, 5, 9, 7, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	5, 8, 4, 5, 10, 8, 10, 11, 8, -1, -1, -1, -1, -1, -1,
	5, 0, 4, 5, 11, 0, 5, 10, 11, 11, 3, 0, -1, -1, -1,
	0, 1, 9, 8, 4, 10, 8, 10, 11, 10, 4, 5, -1, -1, -1,
	10, 11, 4, 10, 4, 5, 11, 3, 4, 9, 4, 1, 3, 1, 4,
	2, 5, 1, 2, 8, 5, 2, 11, 8, 4, 5, 8, -1, -1, -1,
	0, 4, 11, 0, 11, 3, 4, 5, 11, 2, 11, 1, 5, 1, 11,
	0, 2, 5, 0, 5, 9, 2, 11, 5, 4, 5, 8, 11, 8, 5,
	9, 4, 5, 2, 11, 3, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	2, 5, 10, 3, 5, 2, 3, 4, 5, 3, 8, 4, -1, -1, -1,
	5, 10, 2, 5, 2, 4, 4, 2, 0, -1, -1, -1, -1, -1, -1,
	3, 10, 2, 3, 5, 10, 3, 8, 5, 4, 5, 8, 0, 1, 9,
	5, 10, 2, 5, 2, 4, 1, 9, 2, 9, 4, 2, -1, -1, -1,
	8, 4, 5, 8, 5, 3, 3, 5, 1, -1, -1, -1, -1, -1, -1,
	0, 4, 5, 1, 0, 5, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	8, 4, 5, 8, 5, 3, 9, 0, 5, 0, 3, 5, -1, -1, -1,
	9, 4, 5, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	4, 11, 7, 4, 9, 11, 9, 10, 11, -1, -1, -1, -1, -1, -1,
	0, 8, 3, 4, 9, 7, 9, 11, 7, 9, 10, 11, -1, -1, -1,
	1, 10, 11, 1, 11, 4, 1, 4, 0, 7, 4, 11, -1, -1, -1,
	3, 1, 4, 3, 4, 8, 1, 10, 4, 7, 4, 11, 10, 11, 4,
	4, 11, 7, 9, 11, 4, 9, 2, 11, 9, 1, 2, -1, -1, -1,
	9, 7, 4, 9, 11, 7, 9, 1, 11, 2, 11, 1, 0, 8, 3,
	11, 7, 4, 11, 4, 2, 2, 4, 0, -1, -1, -1, -1, -1, -1,
	11, 7, 4, 11, 4, 2, 8, 3, 4, 3, 2, 4, -1, -1, -1,
	2, 9, 10, 2, 7, 9, 2, 3, 7, 7, 4, 9, -1, -1, -1,
	9, 10, 7, 9, 7, 4, 10, 2, 7, 8, 7, 0, 2, 0, 7,
	3, 7, 10, 3, 10, 2, 7, 4, 10, 1, 10, 0, 4, 0, 10,
	1, 10, 2, 8, 7, 4, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	4, 9, 1, 4, 1, 7, 7, 1, 3, -1, -1, -1, -1, -1, -1,
	4, 9, 1, 4, 1, 7, 0, 8, 1, 8, 7, 1, -1, -1, -1,
	4, 0, 3, 7, 4, 3, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	4, 8, 7, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	9, 10, 8, 10, 11, 8, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	3, 0, 9, 3, 9, 11, 11, 9, 10, -1, -1, -1, -1, -1, -1,
	0, 1, 10, 0, 10, 8, 8, 10, 11, -1, -1, -1, -1, -1, -1,
	3, 1, 10, 11, 3, 10, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	1, 2, 11, 1, 11, 9, 9, 11, 8, -1, -1, -1, -1, -1, -1,
	3, 0, 9, 3, 9, 11, 1, 2, 9, 2, 11, 9, -1, -1, -1,
	0, 2, 11, 8, 0, 11, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	3, 2, 11, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	2, 3, 8, 2, 8, 10, 10, 8, 9, -1, -1, -1, -1, -1, -1,
	9, 10, 2, 0, 9, 2, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	2, 3, 8, 2, 8, 10, 0, 1, 8, 1, 10, 8, -1, -1, -1,
	1, 10, 2, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	1, 3, 8, 9, 1, 8, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	0, 9, 1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	0, 3, 8, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1};

GLuint Node_AddSurface::triBuf, Node_AddSurface::triBufT;