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
	bool visible;
	string name;
	Residue* residues;
	uint residueSz;
};

class Particles {
public:
	ResidueList* residueLists;
	uint residueListSz;
	Particle* particles;
	uint particleSz;
};