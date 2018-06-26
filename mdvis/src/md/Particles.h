#pragma once
#include "Engine.h"
#include "vis/system.h"

const uint PAR_MAX_NAME_LEN = 6;

struct Residue {
	Residue() : visible(true), expanded(false), drawType(0x22) {}

	uint maxOff;
	bool visible, expanded;
	string name;
	uint offset, offset_b;
	ushort cnt, cnt_b;
	byte type, drawType;
	bool selected;
};

struct ResidueList { //residues with the same name
	ResidueList() : visible(true), expanded(false), drawType(0x22), selected(false) {}

	~ResidueList() {
		if (residues) std::free(residues);
	}
	
	uint maxOff;
	bool visible, expanded;
	string name;
	Residue* residues = 0;
	uint residueSz = 0;
	byte drawType;
	bool selected;
};

struct AnimData {
	AnimData () : reading(false), frameCount(0), activeFrame(0) {}

	bool reading, dynamicBonds;
	uint frameCount, activeFrame;
	Vec3** poss, **vels;
	uint* connCounts;
	Int2** conns;
	std::vector<std::pair<uint*, Int2**>> conns2;
	
private:
	AnimData(const AnimData&);
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
	static float* particles_Rad;
	static Int2* particles_Res;

	struct conn2info {
		uint cnt, ocnt = 0;
		Int2* ids;
		GLuint buf = 0, tbuf = 0;
		byte drawMode;
		bool visible;
	};
	static std::vector<conn2info> particles_Conn2;

	static void UpdateConBufs2();

	static AnimData anim;
	static void IncFrame(bool loop);
	static void SetFrame(uint frm);

	static Vec3 boundingBox;

	static Vec3 colorPallete[256];
	static ushort defColPallete[256];
	static byte defColPalleteSz;
	static GLuint colorPalleteTex;
	static bool palleteDirty;

	static void Init(), Clear(), GenTexBufs(), UpdateBufs(), UpdateColorTex(), UpdateRadBuf();

	static GLuint posVao;
	static GLuint posBuffer; //xyz
	static GLuint connBuffer; //uint uint
	static GLuint colIdBuffer; //byte
	static GLuint radBuffer; //float
	static GLuint posTexBuffer, connTexBuffer, colorIdTexBuffer, radTexBuffer;
};