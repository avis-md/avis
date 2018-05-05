#pragma once
#include "Engine.h"

/*
struct Particle {
	bool visible;
	string name;
	Vec3 position, velocity;
	byte colorKey;
	byte conns[4];
};
*/
struct Residue {
	Residue() : visible(true), expanded(true) {}

	uint maxOff;
	bool visible, expanded;
	string name;
	uint offset;
	ushort cnt;
};

struct ResidueList { //residues with the same name
	ResidueList() : visible(true), expanded(true) {}

	~ResidueList() {
		delete[](residues);
	}
	
	uint maxOff;
	bool visible, expanded;
	string name;
	Residue* residues;
	uint residueSz;
};

class Particles {
public:
	static ResidueList* residueLists;
	static uint residueListSz;
	static uint particleSz;
	static uint connSz;
	
	static string* particles_Name, *particles_ResName;
	static Vec3* particles_Pos, *particles_Vel;
	static byte* particles_Col;

	static Vec3 colorPallete[256];
	static GLuint colorPalleteTex;

	static void Init(), Clear(), GenTexBufs(), UpdateColorTex();

	static GLuint posVao;
	static GLuint posBuffer; //xyz
	static GLuint connBuffer; //uint uint
	static GLuint colIdBuffer; //byte
	static GLuint posTexBuffer, connTexBuffer, colorIdTexBuffer;
};