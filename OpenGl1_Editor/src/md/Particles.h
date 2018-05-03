#pragma once
#include "Engine.h"

struct Particle {
	bool visible;
	string name;
	Vec3 position, velocity;
	byte colorKey;
	byte conns[4];
};

struct Residue {
	bool visible;
	uintptr_t offset;
	ushort cnt;
};

struct ResidueList { //residues with the same name
	~ResidueList() {
		delete[](residues);
	}
	
	bool visible;
	string name;
	Residue* residues;
	uint residueSz;
};

class Particles {
public:
	static ResidueList* residueLists;
	static uint residueListSz;
	static Particle* particles;
	static uint particleSz;
	static uint connSz;
	
	static GLuint colorPalleteTex;

	static void Init(), Clear(), GenTexBufs();

	static GLuint posVao;
	static GLuint posBuffer; //xyz
	static GLuint connBuffer; //uint uint
	static GLuint colIdBuffer; //byte
	static GLuint posTexBuffer, connTexBuffer, colorIdTexBuffer;
};