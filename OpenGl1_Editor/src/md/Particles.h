#pragma once
#include "Engine.h"

const uint PAR_MAX_NAME_LEN = 10;

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
	Residue() : visible(true), expanded(false) {}

	uint maxOff;
	bool visible, expanded;
	string name;
	uint offset, offset_b;
	ushort cnt, cnt_b;
};

struct ResidueList { //residues with the same name
	ResidueList() : visible(true), expanded(false) {}

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
	
	static char* particles_Name, *particles_ResName; //10 chars per name
	static Vec3* particles_Pos, *particles_Vel;
	static byte* particles_Col;
	static Int2* particles_Conn;

	static Vec3 colorPallete[256];
	static GLuint colorPalleteTex;

	static void Init(), Clear(), GenTexBufs(), UpdateColorTex();

	static GLuint posVao;
	static GLuint posBuffer; //xyz
	static GLuint connBuffer; //uint uint
	static GLuint colIdBuffer; //byte
	static GLuint posTexBuffer, connTexBuffer, colorIdTexBuffer;
};