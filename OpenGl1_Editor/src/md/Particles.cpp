#include "Particles.h"

ResidueList* Particles::residueLists;
uint Particles::residueListSz;
Particle* Particles::particles;
uint Particles::particleSz;
uint Particles::connSz;

GLuint Particles::colorPalleteTex;

GLuint Particles::posVao;
GLuint Particles::posBuffer;
GLuint Particles::connBuffer;
GLuint Particles::colIdBuffer;
GLuint Particles::posTexBuffer, Particles::connTexBuffer, Particles::colorIdTexBuffer;

void Particles::Init() {
	byte data[768] = {};
	data[0] = data[4] = data[8] = 255;
	data[1] = data[2] = data[3] = data[5] = data[6] = data[7] = 85;

	glGenTextures(1, &colorPalleteTex);
	glBindTexture(GL_TEXTURE_2D, colorPalleteTex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 16, 16, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, 1);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glBindTexture(GL_TEXTURE_2D, 0);
}

void Particles::Clear() {
	if (particles) {
		delete[](residueLists);
		delete[](particles);
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