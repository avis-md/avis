#include "node_addsurface.h"
#include "vis/pargraphics.h"

bool Node_AddSurface::initd = false;
PROGDEF(Node_AddSurface::marcherProg);
PROGDEF(Node_AddSurface::drawProg);

size_t Node_AddSurface::bufSz = 0, Node_AddSurface::outSz = 0;
uint Node_AddSurface::genSz = 0;
GLuint Node_AddSurface::inBuf, Node_AddSurface::vao, Node_AddSurface::outPos, Node_AddSurface::outNrm, Node_AddSurface::query;

std::mutex Node_AddSurface::lock;

Node_AddSurface::Node_AddSurface() : AnNode(new DmScript(sig)) {
	if (!initd) Init();

	title = "Draw Surface";
	titleCol = NODE_COL_MOD;

	AddInput();
	script->AddInput("density", "list(3d)");
	AddInput();
	script->AddInput("value", "double");
}

Node_AddSurface::~Node_AddSurface() {

}

void Node_AddSurface::Execute() {
	//
	std::lock_guard<std::mutex> locker(lock);
	data.resize(4);
	data[0] = 1;
	data[1] = 2;
	data[2] = 3;
	data[3] = 4;
}

void Node_AddSurface::Update() {
	if (!!data.size()) {
		std::lock_guard<std::mutex> locker(lock);
		genSz = 0;
		Set();
		ExecMC();
		data.clear();
	}
}

void Node_AddSurface::DrawScene() {
	if (!genSz) return;

	glUseProgram(drawProg);
	glUniformMatrix4fv(drawProgLocs[0], 1, GL_FALSE, glm::value_ptr(ParGraphics::lastMVP));
	glBindVertexArray(vao);
	glDrawArrays(GL_TRIANGLES, 0, genSz*3);
	glBindVertexArray(0);
	glUseProgram(0);
}

void Node_AddSurface::Init() {
	GLuint vs, gs;
	std::string err;
	if (!Shader::LoadShader(GL_VERTEX_SHADER, IO::GetText(IO::path + "surfVert.glsl"), vs, &err)) {
		Debug::Error("AddSurface::Init", "Failed to load vertex shader! " + err);
		return;
	}
	if (!Shader::LoadShader(GL_GEOMETRY_SHADER, IO::GetText(IO::path + "surfGeom.glsl"), gs, &err)) {
		Debug::Error("AddSurface::Init", "Failed to load geometry shader! " + err);
		return;
	}

	marcherProg = glCreateProgram();
	glAttachShader(marcherProg, vs);
	glAttachShader(marcherProg, gs);
	const char* fb[] = { "outPos", "outNrm" };
	glTransformFeedbackVaryings(marcherProg, 2, fb, GL_SEPARATE_ATTRIBS);
	int link_result = 0;
	glLinkProgram(marcherProg);
	glGetProgramiv(marcherProg, GL_LINK_STATUS, &link_result);
	if (!link_result)
	{
		int info_log_length = 0;
		glGetProgramiv(marcherProg, GL_INFO_LOG_LENGTH, &info_log_length);
		std::vector<char> program_log(info_log_length);
		glGetProgramInfoLog(marcherProg, info_log_length, NULL, &program_log[0]);
		Debug::Error("AddSurface::Init", "Link error: " + std::string(program_log.data(), info_log_length));
		glDeleteProgram(marcherProg);
		marcherProg = 0;
		return;
	}

	glDetachShader(marcherProg, vs);
	glDetachShader(marcherProg, gs);
	glDeleteShader(vs);
	glDeleteShader(gs);

	//drawProg = Shader::FromVF(IO::GetText(IO::path + "surfDVert.glsl"), IO::GetText(IO::path + "surfDFrag.glsl"));
	//int i = 0;
#define LOC(nm) drawProgLocs[i++] = glGetUniformLocation(drawProg, #nm)
	//LOC(_MVP);
#undef LOC

	glGenBuffers(1, &inBuf);
	glGenVertexArrays(1, &vao);
	glGenBuffers(1, &outPos);
	glGenBuffers(1, &outNrm);
	glBindBuffer(GL_ARRAY_BUFFER, outPos);
	glBufferData(GL_ARRAY_BUFFER, 12 * sizeof(Vec4), nullptr, GL_STATIC_READ);
	glBindBuffer(GL_ARRAY_BUFFER, outNrm);
	glBufferData(GL_ARRAY_BUFFER, 12 * sizeof(Vec4), nullptr, GL_STATIC_READ);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	bufSz = 4;
	outSz = 12;
	/*
	glBindVertexArray(vao);
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glBindBuffer(GL_ARRAY_BUFFER, outPos);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, NULL);
	glBindBuffer(GL_ARRAY_BUFFER, outNrm);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 0, NULL);
	glBindVertexArray(0);
	*/
	glGenQueries(1, &query);
	initd = true;
}

void Node_AddSurface::Set() {
	const auto sz = data.size();
	if (sz != bufSz) {
		bufSz = sz;
		outSz = sz * 3;
		glBindBuffer(GL_ARRAY_BUFFER, inBuf);
		glBufferData(GL_ARRAY_BUFFER, bufSz, data.data(), GL_STATIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, outPos);
		glBufferData(GL_ARRAY_BUFFER, outSz * sizeof(Vec4), nullptr, GL_STATIC_READ);
		glBindBuffer(GL_ARRAY_BUFFER, outNrm);
		glBufferData(GL_ARRAY_BUFFER, outSz * sizeof(Vec4), nullptr, GL_STATIC_READ);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		
		/*
		glBindVertexArray(vao);
		glEnableVertexAttribArray(0);
		glEnableVertexAttribArray(1);
		glBindBuffer(GL_ARRAY_BUFFER, outPos);
		glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, NULL);
		glBindBuffer(GL_ARRAY_BUFFER, outNrm);
		glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 0, NULL);
		glBindVertexArray(0);
		*/
	}
	else {
		glBindBuffer(GL_ARRAY_BUFFER, inBuf);
		glBufferSubData(GL_ARRAY_BUFFER, 0, bufSz, data.data());
		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}
}

void Node_AddSurface::ExecMC() {
	glUseProgram(marcherProg);
	glBindVertexArray(Camera::emptyVao);
	glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, outPos);
	glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 1, outNrm);
	glEnable(GL_RASTERIZER_DISCARD);
	glBeginQuery(GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN, query);
	glBeginTransformFeedback(GL_TRIANGLES);
	glDrawArrays(GL_POINTS, 0, bufSz);
	glEndTransformFeedback();
	glEndQuery(GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN);
	glGetQueryObjectuiv(query, GL_QUERY_RESULT, &genSz);
	glDisable(GL_RASTERIZER_DISCARD);
	glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, 0);
	glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 1, 0);
	glBindVertexArray(0);
	glUseProgram(0);

	std::vector<Vec4> poss(outSz);
	glBindBuffer(GL_TRANSFORM_FEEDBACK_BUFFER, outPos);
	glGetBufferSubData(GL_TRANSFORM_FEEDBACK_BUFFER, 0, outSz * sizeof(Vec4), poss.data());
	glBindBuffer(GL_TRANSFORM_FEEDBACK_BUFFER, 0);
	for (auto& a : poss) std::cout << std::to_string(a) << std::endl;
}