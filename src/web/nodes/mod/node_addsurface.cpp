#include "node_addsurface.h"
#include "vis/pargraphics.h"
#include "md/particles.h"
#include "res/shd/marchVert.h"
#include "res/shd/marchGeom.h"

INODE_DEF(__("Draw Surface"), AddSurface, GEN);

Shader Node_AddSurface::marchProg;
Shader Node_AddSurface::drawProg;

size_t Node_AddSurface::bufSz = 0, Node_AddSurface::outSz = 0;
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
	);

	glGenVertexArrays(1, &vao);
	glGenBuffers(1, &outPos);
	glGenBuffers(1, &outNrm);

	glGetIntegerv(GL_MAX_TEXTURE_BUFFER_SIZE, &maxBufSz);
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
		Set();
		ExecMC();
		data.clear();
		if (!!genSz) Scene::dirty = true;
	}
}

void Node_AddSurface::DrawScene(RENDER_PASS pass) {
	if (!genSz) return;

	if (pass == RENDER_PASS::SOLID) {
		auto e = glGetError();
		glUseProgram(drawProg);
		glUniformMatrix4fv(drawProg.Loc(0), 1, GL_FALSE, glm::value_ptr(MVP::modelview()));
		glUniformMatrix4fv(drawProg.Loc(1), 1, GL_FALSE, glm::value_ptr(MVP::projection() * MVP::modelview()));
		auto& bboxs = Particles::boundingBox;
		glUniform3f(drawProg.Loc(2), bboxs[0], bboxs[2], bboxs[4]);
		glUniform3f(drawProg.Loc(3), bboxs[1], bboxs[3], bboxs[5]);
		glBindVertexArray(vao);
		glDrawArrays(GL_TRIANGLES, 0, genSz*3);
		e = glGetError();
		glBindVertexArray(0);
		glUseProgram(0);
	}
}

void Node_AddSurface::RayTraceMesh(_Mesh& mesh) {
	if (!genSz) return;
	mesh.vertCount = (mesh.triCount = genSz) * 3;
	mesh.vertices.resize(mesh.vertCount);
	mesh.normals.resize(mesh.vertCount);
	mesh.triangles.resize(mesh.vertCount);
	glBindBuffer(GL_ARRAY_BUFFER, outPos);
	auto pos = (Vec4*)glMapBufferRange(GL_ARRAY_BUFFER, 0, genSz * sizeof(Vec4), GL_MAP_READ_BIT);
	for (uint a = 0; a < mesh.vertCount; a++) {
		mesh.vertices[a] = pos[a] * 5.f;
	}
	glUnmapBuffer(GL_ARRAY_BUFFER);
	glBindBuffer(GL_ARRAY_BUFFER, outNrm);
	auto nrm = (Vec4*)glMapBufferRange(GL_ARRAY_BUFFER, 0, genSz * sizeof(Vec4), GL_MAP_READ_BIT);
	for (uint a = 0; a < mesh.vertCount; a++) {
		mesh.normals[a] = -nrm[a];
	}
	glUnmapBuffer(GL_ARRAY_BUFFER);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	for (uint a = 0; a < mesh.vertCount; a++) {
		mesh.triangles[a] = a;
	}
}

void Node_AddSurface::Execute() {
	genSz = 0;
	auto& ir = inputR[0];
	if (!ir.first) return;
	auto& cv = ir.getconv();
	auto& vl = *(double**)ir.getval(ANVAR_ORDER::C);
	if (!vl) return;
	shape[0] = ir.getdim(0);
	shape[1] = ir.getdim(1);
	shape[2] = ir.getdim(2);
	const int sz = shape[0] * shape[1] * shape[2];
	if (!sz) return;
	if (sz > maxBufSz) {
		Debug::Warning("AddSurface", "Exceeded maximum allowed texel size! ("
			+ std::to_string(bufSz) + " required, " + std::to_string(maxBufSz) + " available)");
		return;
	}
	std::lock_guard<std::mutex> locker(lock);
	data.resize(sz);
#pragma omp parallel for
	for (int a = 0; a < sz; a++) {
		data[a] = (float)vl[a];
	}
	cutoff = (float)getval_d(1);
}

void Node_AddSurface::Init() {
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
	int link_result = 0;
	glLinkProgram(marchProg);
	glGetProgramiv(marchProg, GL_LINK_STATUS, &link_result);
	if (!link_result)
	{
		int info_log_length = 0;
		glGetProgramiv(marchProg, GL_INFO_LOG_LENGTH, &info_log_length);
		std::vector<char> program_log(info_log_length);
		glGetProgramInfoLog(marchProg, info_log_length, NULL, &program_log[0]);
		Debug::Error("AddSurface::Init", "Link error: " + std::string(program_log.data(), info_log_length));
		marchProg = 0;
		return;
	}

	glDetachShader(marchProg, vs);
	glDetachShader(marchProg, gs);
	glDeleteShader(vs);
	glDeleteShader(gs);

	marchProg.AddUniforms({ "data", "val", "shp", "triBuf" });

	(drawProg = Shader::FromVF(IO::GetText(IO::path + "shaders/surfDVert.glsl"), IO::GetText(IO::path + "shaders/surfDFrag.glsl")))
		.AddUniforms({ "_MV", "_MVP", "bbox1", "bbox2" });

	glGenBuffers(1, &inBuf);
	glGenTextures(1, &inBufT);
	glGenQueries(1, &query);

	glGenBuffers(1, &triBuf);
	glGenTextures(1, &triBufT);
	glBindBuffer(GL_ARRAY_BUFFER, triBuf);
	glBufferData(GL_ARRAY_BUFFER, sizeof(triTable), triTable, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindTexture(GL_TEXTURE_BUFFER, triBufT);
	glTexBuffer(GL_TEXTURE_BUFFER, GL_R32F, triBuf);
	glBindTexture(GL_TEXTURE_BUFFER, 0);
}

void Node_AddSurface::Set() {
	bufSz = data.size();
	outSz = (shape[0]-1)*(shape[1]-1)*(shape[2]-1) * 15;
	glBindBuffer(GL_ARRAY_BUFFER, inBuf);
	glBufferData(GL_ARRAY_BUFFER, bufSz * sizeof(float), data.data(), GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, outPos);
	glBufferData(GL_ARRAY_BUFFER, outSz * sizeof(Vec4), nullptr, GL_STATIC_READ);
	glBindBuffer(GL_ARRAY_BUFFER, outNrm);
	glBufferData(GL_ARRAY_BUFFER, outSz * sizeof(Vec4), nullptr, GL_STATIC_READ);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glBindTexture(GL_TEXTURE_BUFFER, inBufT);
	glTexBuffer(GL_TEXTURE_BUFFER, GL_R32F, inBuf);
	glBindTexture(GL_TEXTURE_BUFFER, 0);
	glBindVertexArray(vao);
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glBindBuffer(GL_ARRAY_BUFFER, outPos);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, NULL);
	glBindBuffer(GL_ARRAY_BUFFER, outNrm);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 0, NULL);
	glBindVertexArray(0);
}

void Node_AddSurface::ExecMC() {
	glUseProgram(marchProg);
	glBindVertexArray(Camera::emptyVao);
	glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, outPos);
	glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 1, outNrm);

	glUniform1i(marchProg.Loc(0), 1);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_BUFFER, inBufT);
	glUniform1f(marchProg.Loc(1), cutoff);
	glUniform3i(marchProg.Loc(2), shape[0], shape[1], shape[2]);
	glUniform1i(marchProg.Loc(3), 2);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_BUFFER, triBufT);

	glEnable(GL_RASTERIZER_DISCARD);
	glBeginQuery(GL_PRIMITIVES_GENERATED, query);
	glBeginTransformFeedback(GL_TRIANGLES);
	glDrawArrays(GL_POINTS, 0, (shape[0]-1)*(shape[1]-1)*(shape[2]-1));
	glEndTransformFeedback();
	glEndQuery(GL_PRIMITIVES_GENERATED);
	glGetQueryObjectuiv(query, GL_QUERY_RESULT, &genSz);
	glDisable(GL_RASTERIZER_DISCARD);
	glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, 0);
	glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 1, 0);
	glBindVertexArray(0);
	glUseProgram(0);
}

const float Node_AddSurface::triTable[] = {
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