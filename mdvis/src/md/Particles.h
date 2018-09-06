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
	uint cnt, cnt_b;
	byte type, drawType;
	bool selected;
};

struct ResidueList { //residues with the same name
	ResidueList() : visible(true), expanded(false), drawType(0x22), selected(false) {}
	
	uint maxOff;
	bool visible, visibleAll = true, expanded;
	string name;
	std::vector<Residue> residues;
	uint residueSz = 0;
	byte drawType;
	bool selected;
};

struct AnimData {
	AnimData () : reading(false), frameCount(0), activeFrame(0) {}

	bool reading, dynamicBonds;
	uint frameCount, activeFrame;
	glm::dvec3** poss, **vels;
	uint* connCounts;
	Int2** conns;
	std::vector<std::pair<uint*, Int2**>> conns2;
	
private:
	AnimData(const AnimData&);
};

class Particles {
public:
	struct conninfo {
		uint cnt, ocnt = 0;
		Int2* ids;
		GLuint buf = 0, tbuf = 0;
		byte drawMode = 1;
		float scale = 1;
		bool dashed = false;
		float line_sc = 1, line_sp = 0.5f, line_rt = 0.5f;
		bool usecol = false;
		Vec4 col = white();
		bool visible = true;
	};

	struct paramdata {
		paramdata();
		~paramdata();

		bool dirty = false;
		float* data;
		GLuint buf, texBuf;
		
		void Update();
	};

	static std::vector<ResidueList> residueLists;
	static uint residueListSz;
	static uint particleSz;

	static char* particles_Name, *particles_ResName; //10 chars per name
	static glm::dvec3* particles_Pos, *particles_Vel;
	static short* particles_Typ;
	static byte* particles_Col;
	static conninfo particles_Conn;
	static float* particles_Rad;
	static Int2* particles_Res;

	static int particles_ParamSz;
	static paramdata* particles_Params[10];
	static string particles_ParamNms[11];

	static std::vector<conninfo> particles_Conn2;

	static void UpdateConBufs2();

	static AnimData anim;
	static void IncFrame(bool loop);
	static void SetFrame(uint frm);

	static float boundingBox[6];

	static Vec3 colorPallete[256];
	static ushort defColPallete[256];
	static Vec4 _colorPallete[256];
	static byte defColPalleteSz;
	static GLuint colorPalleteTex;
	static bool palleteDirty;

	static void Init(), Clear(), GenTexBufs(), UpdateBufs(), UpdateColorTex(), UpdateRadBuf();
	static void AddParam(), RmParam(int i);

	static GLuint posVao;
	static GLuint posBuffer; //xyz
	static GLuint connBuffer; //uint uint
	static GLuint colIdBuffer; //byte
	static GLuint radBuffer; //float
	static GLuint posTexBuffer, connTexBuffer, colorIdTexBuffer, radTexBuffer;
};