#include "Particles.h"

ResidueList* Particles::residueLists;
uint Particles::residueListSz;
uint Particles::particleSz;
uint Particles::connSz;

char* Particles::particles_Name, *Particles::particles_ResName;
Vec3* Particles::particles_Pos, *Particles::particles_Vel;
byte* Particles::particles_Col;
Int2* Particles::particles_Conn;

Vec3 Particles::boundingBox;

Vec3 Particles::colorPallete[] = {};
GLuint Particles::colorPalleteTex;

GLuint Particles::posVao;
GLuint Particles::posBuffer;
GLuint Particles::connBuffer;
GLuint Particles::colIdBuffer;
GLuint Particles::posTexBuffer, Particles::connTexBuffer, Particles::colorIdTexBuffer;

void Particles::Init() {
	colorPallete[0].r = colorPallete[1].g = colorPallete[2].b = 1;
	colorPallete[0].g = colorPallete[0].b = colorPallete[1].r = colorPallete[1].b = colorPallete[2].r = colorPallete[2].g = 0;

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

	glBindTexture(GL_TEXTURE_BUFFER, 0);
}