#include "Particles.h"
#include "web/anweb.h"
#include "md/Protein.h"

ResidueList* Particles::residueLists;
uint Particles::residueListSz;
uint Particles::particleSz;
uint Particles::connSz;

char* Particles::particles_Name, *Particles::particles_ResName;
Vec3* Particles::particles_Pos, *Particles::particles_Vel;
byte* Particles::particles_Col;
Int2* Particles::particles_Conn;
float* Particles::particles_Rad;
Int2* Particles::particles_Res;

std::vector<Particles::conn2info> Particles::particles_Conn2;

AnimData Particles::anim;

float Particles::boundingBox[] = {};

Vec3 Particles::colorPallete[] = {};
ushort Particles::defColPallete[] = {};
byte Particles::defColPalleteSz = 0;
GLuint Particles::colorPalleteTex;
bool Particles::palleteDirty = false;

GLuint Particles::posVao;
GLuint Particles::posBuffer;
GLuint Particles::connBuffer;
GLuint Particles::colIdBuffer;
GLuint Particles::radBuffer;
GLuint Particles::posTexBuffer, Particles::connTexBuffer, Particles::colorIdTexBuffer, Particles::radTexBuffer;

void Particles::Init() {
	glGenVertexArrays(1, &posVao);
	glGenBuffers(1, &posBuffer);
	glGenBuffers(1, &connBuffer);
	glGenBuffers(1, &colIdBuffer);
	glGenBuffers(1, &radBuffer);

	glBindVertexArray(posVao);
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, posBuffer);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	glGenTextures(1, &colorPalleteTex);
	glBindTexture(GL_TEXTURE_2D, colorPalleteTex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 16, 16, 0, GL_RGB, GL_FLOAT, colorPallete);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glBindTexture(GL_TEXTURE_2D, 0);
}

void Particles::UpdateColorTex() {
	glBindTexture(GL_TEXTURE_2D, colorPalleteTex);
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 16, 16, GL_RGB, GL_FLOAT, colorPallete);
	glBindTexture(GL_TEXTURE_2D, 0);
}

void Particles::Clear() {
	if (particles_Pos) {
		delete[](residueLists);
		delete[](particles_Name);
		delete[](particles_ResName);
		delete[](particles_Pos);
		delete[](particles_Vel);
		delete[](particles_Col);
		delete[](particles_Conn);
		glDeleteBuffers(1, &posBuffer);
		glDeleteBuffers(1, &connBuffer);
		glDeleteVertexArrays(1, &posVao);
		glDeleteTextures(1, &posTexBuffer);
		glDeleteTextures(1, &connTexBuffer);
		Protein::Clear();
	}
}

void Particles::GenTexBufs() {
	glGenTextures(1, &posTexBuffer);
	glBindTexture(GL_TEXTURE_BUFFER, posTexBuffer);
	glTexBuffer(GL_TEXTURE_BUFFER, GL_RGB32F, posBuffer);
	
	glGenTextures(1, &connTexBuffer);
	glBindTexture(GL_TEXTURE_BUFFER, connTexBuffer);
	glTexBuffer(GL_TEXTURE_BUFFER, GL_R32UI, connBuffer);

	glGenTextures(1, &colorIdTexBuffer);
	glBindTexture(GL_TEXTURE_BUFFER, colorIdTexBuffer);
	glTexBuffer(GL_TEXTURE_BUFFER, GL_R8, colIdBuffer);

	glGenTextures(1, &radTexBuffer);
	glBindTexture(GL_TEXTURE_BUFFER, radTexBuffer);
	glTexBuffer(GL_TEXTURE_BUFFER, GL_R32F, radBuffer);

	glBindTexture(GL_TEXTURE_BUFFER, 0);
}

void Particles::UpdateBufs() {
	glBindBuffer(GL_ARRAY_BUFFER, posBuffer);
	glBufferData(GL_ARRAY_BUFFER, particleSz * sizeof(Vec3), particles_Pos, GL_DYNAMIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, connBuffer);
	glBufferData(GL_ARRAY_BUFFER, connSz * 2 * sizeof(uint), particles_Conn, GL_DYNAMIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, colIdBuffer);
	glBufferData(GL_ARRAY_BUFFER, particleSz * sizeof(byte), particles_Col, GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, radBuffer);
	glBufferData(GL_ARRAY_BUFFER, particleSz * sizeof(float), particles_Rad, GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void Particles::UpdateRadBuf() {
	glBindBuffer(GL_ARRAY_BUFFER, radBuffer);
	glBufferSubData(GL_ARRAY_BUFFER, 0, particleSz * sizeof(float), particles_Rad);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void Particles::UpdateConBufs2() {
	for (auto& c2 : particles_Conn2) {
		if (!c2.cnt) continue;
		if (!c2.buf) glGenBuffers(1, &c2.buf);
		glBindBuffer(GL_ARRAY_BUFFER, c2.buf);
		if (c2.ocnt < c2.cnt)
			glBufferData(GL_ARRAY_BUFFER, c2.cnt * sizeof(Int2), c2.ids, GL_DYNAMIC_DRAW);
		else
			glBufferSubData(GL_ARRAY_BUFFER, 0, c2.cnt * sizeof(Int2), c2.ids);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		c2.ocnt = c2.cnt;
		if (!c2.tbuf) {
			glGenTextures(1, &c2.tbuf);
		}
		glBindTexture(GL_TEXTURE_BUFFER, c2.tbuf);
		glTexBuffer(GL_TEXTURE_BUFFER, GL_R32F, c2.buf);
		glBindTexture(GL_TEXTURE_BUFFER, 0);
	}
}

void Particles::IncFrame(bool loop) {
	if (anim.activeFrame >= anim.frameCount - 1) {
		if (loop) SetFrame(0);
		else return;
	}
	else SetFrame(anim.activeFrame + 1);
}

void Particles::SetFrame(uint frm) {
	if (frm == anim.activeFrame) return;
	else {
		anim.activeFrame = frm;
		particles_Pos = anim.poss[anim.activeFrame];
		glBindBuffer(GL_ARRAY_BUFFER, posBuffer);
		glBufferSubData(GL_ARRAY_BUFFER, 0, particleSz * sizeof(Vec3), particles_Pos);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		bool has = false;
		for (int i = anim.conns2.size() - 1; i >= 0; i--) {
			auto& c2 = anim.conns2[i];
			if (!c2.first) continue;
			auto& c = particles_Conn2[i];
			c.cnt = c2.first[frm];
			c.ids = c2.second[frm];
			has = true;
		}
		if (has) UpdateConBufs2();
		AnWeb::OnAnimFrame();
		Scene::active->dirty = true;
	}
}