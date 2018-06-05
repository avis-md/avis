#include "node_volume.h"
#include "../anweb.h"
#include "ui/icons.h"

const Vec3 Node_Volume::cubeVerts[] = {
	Vec3(-1, -1, -1), Vec3(1, -1, -1), Vec3(-1, 1, -1), Vec3(1, 1, -1),
	Vec3(-1, -1, 1), Vec3(1, -1, 1), Vec3(-1, 1, 1), Vec3(1, 1, 1) };
const uint Node_Volume::cubeIndices[] = {
	0, 1, 2, 1, 3, 2,	4, 5, 6, 5, 7, 6,
	0, 2, 4, 2, 6, 4,	1, 3, 5, 3, 7, 5,
	0, 1, 4, 1, 5, 4,	2, 3, 6, 3, 7, 6 };

GLuint Node_Volume::shad, Node_Volume::shadC;
GLint Node_Volume::shadLocs[], Node_Volume::shadCLocs[];
GLuint Node_Volume::vao, Node_Volume::vbo, Node_Volume::veo;

void Node_Volume::Init() {
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, 8 * sizeof(Vec3), cubeVerts, GL_STATIC_DRAW);
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
	glGenBuffers(1, &veo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, veo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, 36 * sizeof(uint), cubeIndices, GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

#define LC(i, nm) shadLocs[i] = glGetUniformLocation(shad, #nm)
	shad = Shader::FromVF(IO::GetText(IO::path + "/voxelVert.txt"), IO::GetText(IO::path + "/voxelFrag.txt"));
	LC(0, _MV);
	LC(1, _P);
	LC(2, size);
	LC(3, tex);
	LC(4, cutPos);
	LC(5, cutDir);
#undef LC
#define LC(i, nm) shadCLocs[i] = glGetUniformLocation(shadC, #nm)
	shadC = Shader::FromVF(IO::GetText(IO::path + "/voxelCutVert.txt"), IO::GetText(IO::path + "/voxelCutFrag.txt"));
	LC(0, _MV);
	LC(1, _P);
}

Node_Volume::Node_Volume() : AnNode(new DmScript()) {
	//width = 200;
	canTile = true;
	inputR.resize(1);
	script->invars.push_back(std::pair<string, string>("array", "list(3)"));
	ox = oy = oz = 4;
	sx = sy = sz = 5;
	cutC = Vec3(0.5f, 0.5f, 0.5f);
	cutD = Vec3(1, 1, 0);
}

void Node_Volume::Draw() {
	auto cnt = 1;
	this->pos = pos;
	Engine::DrawQuad(pos.x, pos.y, width, 16, white(selected ? 1.0f : 0.7f, 0.35f));
	if (Engine::Button(pos.x, pos.y, 16, 16, expanded ? Icons::expand : Icons::collapse, white(0.8f), white(), white(0.5f)) == MOUSE_RELEASE) expanded = !expanded;
	UI::Label(pos.x + 20, pos.y + 1, 12, "Density Plot (DIM=3)", font, white());
	DrawToolbar();
	if (expanded) {
		Engine::DrawQuad(pos.x, pos.y + 16, width, 3 + 17 * (cnt + 5), white(0.7f, 0.25f));
		float y = pos.y + 18;
		const uint i = 0;
		if (!AnWeb::selConnNode || (AnWeb::selConnIdIsOut && AnWeb::selConnNode != this)) {
			if (Engine::Button(pos.x - 5, y + 3, 10, 10, inputR[i].first ? tex_circle_conn : tex_circle_open, white(), white(), white()) == MOUSE_RELEASE) {
				if (!AnWeb::selConnNode) {
					AnWeb::selConnNode = this;
					AnWeb::selConnId = i;
					AnWeb::selConnIdIsOut = false;
					AnWeb::selPreClear = false;
				}
				else {
					AnWeb::selConnNode->ConnectTo(AnWeb::selConnId, this, i);
					AnWeb::selConnNode = nullptr;
				}
			}
		}
		else if (AnWeb::selConnNode == this && AnWeb::selConnId == i && !AnWeb::selConnIdIsOut) {
			Engine::DrawLine(Vec2(pos.x, y + 8), Input::mousePos, white(), 1);
			UI::Texture(pos.x - 5, y + 3, 10, 10, inputR[i].first ? tex_circle_conn : tex_circle_open);
		}
		else {
			UI::Texture(pos.x - 5, y + 3, 10, 10, inputR[i].first ? tex_circle_conn : tex_circle_open, red(0.3f));
		}
		UI::Label(pos.x + 10, y, 12, "values", font, white());
		
	}
}

float Node_Volume::DrawSide() {
	Engine::DrawQuad(pos.x, pos.y, width, 16, white(selected ? 1.0f : 0.7f, 0.35f));
	if (Engine::Button(pos.x, pos.y, 16, 16, expanded ? Icons::expand : Icons::collapse, white(0.8f), white(), white(0.5f)) == MOUSE_RELEASE) expanded = !expanded;
	UI::Label(pos.x + 20, pos.y + 1, 12, "Density Plot", font, white());
	if (expanded) {
		Engine::DrawQuad(pos.x, pos.y + 16, width, 17, white(0.7f, 0.25f));
		
		return 34;
	}
	else return 17;
}

Vec2 Node_Volume::DrawConn() {
	auto cnt = 1;
	float y = pos.y + 18;
	//for (uint i = 0; i < script->invarCnt; i++, y += 17) {
	if (inputR[0].first) Engine::DrawLine(Vec2(pos.x, expanded ? y + 8 : pos.y + 8), Vec2(inputR[0].first->pos.x + inputR[0].first->width, inputR[0].first->expanded ? inputR[0].first->pos.y + 20 + 8 + (inputR[0].second + inputR[0].first->script->invars.size()) * 17 : inputR[0].first->pos.y + 8), white(), 2);
	//}
	if (expanded) return Vec2(width, 19 + 17 * cnt + width);
	else return Vec2(width, 16);
}

void Node_Volume::DrawScene() {
	MVP::Switch(false);
	MVP::Push();
	MVP::Translate(ox, oy, oz);

	glUseProgram(shad);
	glUniformMatrix4fv(shadLocs[0], 1, GL_FALSE, glm::value_ptr(MVP::modelview()));
	glUniformMatrix4fv(shadLocs[1], 1, GL_FALSE, glm::value_ptr(MVP::projection()));
	glUniform3f(shadLocs[2], sx, sy, sz);
	
	glUniform3f(shadLocs[4], cutC.x, cutC.y, cutC.z);
	glUniform3f(shadLocs[5], cutD.x, cutD.y, cutD.z);

	glBindVertexArray(vao);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, veo);
	glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
	glUseProgram(0);

	float cs = 1.0f / sqrtf(2.0f);

	//MVP::Mul(Mat4x4(cs, -cs, 0, 0, cs, cs, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1));
	glUseProgram(shadC);
	glUniformMatrix4fv(shadCLocs[0], 1, GL_FALSE, glm::value_ptr(MVP::modelview()));
	glUniformMatrix4fv(shadCLocs[1], 1, GL_FALSE, glm::value_ptr(MVP::projection()));
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glUseProgram(0);
	MVP::Pop();
}

void Node_Volume::Execute() {
	if (!inputR[0].first) return;
	CVar& cv = inputR[0].first->conV[inputR[0].second];
	auto& sz1 = *cv.dimVals[0];
	auto& sz2 = *cv.dimVals[1];
	auto& sz3 = *cv.dimVals[2];

	if (sz1 != nx || sz2 != ny || sz3 != nz) {
		nx = sz1;
		ny = sz2;
		nz = sz3;
		if (tex) glDeleteTextures(1, &tex);
		glGenTextures(1, &tex);
		glBindTexture(GL_TEXTURE_3D, tex);
		glTexImage3D(GL_TEXTURE_3D, 0, GL_R32F, nx, ny, nz, 0, GL_RED, GL_FLOAT, *((float**)cv.value));
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glBindTexture(GL_TEXTURE_3D, 0);

	}

	float* src = *((float**)cv.value);
}