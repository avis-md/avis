#pragma once
#include "Engine.h"
#include "vis/system.h"

const uint PAR_MAX_NAME_LEN = 6;

struct Residue {
	Residue() : visible(true), expanded(false), drawType(0x22), selected(false) {}

	uint maxOff;
	bool visible, expanded;
	std::string name;
	uint offset, offset_b;
	uint cnt, cnt_b;
	byte type, drawType;
	bool selected;
};

struct ResidueList { //residues with the same name
	ResidueList() : visible(true), expanded(false), drawType(0x22), selected(false) {}
	
	uint maxOff;
	bool visible, visibleAll = true, expanded;
	std::string name;
	std::vector<Residue> residues;
	uint residueSz = 0;
	byte drawType;
	bool selected;
};

class Particles {
public:
	struct conninfo {
		uint cnt, ocnt = 0;
		std::vector<Int2> ids;
		GLuint buf = 0, tbuf = 0;
		byte drawMode = 1;
		float scale = 1;
		bool dashed = false;
		float line_sc = 1, line_sp = 0.5f, line_rt = 0.5f;
		bool usecol = false;
		Vec4 col = white();
		bool visible = true;
	};
	struct conndata {
		uint count;
		std::vector<Int2> ids;
	};
	struct paramdata {
		paramdata();
		~paramdata();

		bool dirty = false;
		bool timed = false;
		std::vector<float> data;
		GLuint buf, texBuf;
		
		void Update();
	};

	static uint residueListSz;
	static uint particleSz, _particleSz;

	static std::string cfgFile, trjFile;

	static glm::dvec3* poss, *vels;

	static std::vector<ResidueList> residueLists;

	static std::vector<char> names, resNames;
	static std::vector<short> types;
	static std::vector<byte> colors;
	static std::vector<float> radii;
	static std::vector<bool> visii;
	static std::vector<Int2> ress;
	static conninfo conns;

	static bool visDirty;

	static int particles_ParamSz;
	static paramdata* particles_Params[10];
	static std::string particles_ParamNms[11];

	static std::vector<conninfo> particles_Conn2;

	static void UpdateConBufs2();

	struct AnimData {
		enum class FRAME_STATUS {
			UNLOADED,
			READING,
			LOADED,
			BAD
		};

		AnimData () {}

		bool reading = false, dynamicBonds;
		static uint maxFramesInMem;
		uint frameMemPos;
		uint frameCount = 0, currentFrame = 0;
		std::vector<std::string> paths;
		std::vector<FRAME_STATUS> status;
		std::vector<std::vector<glm::dvec3>> poss, vels;
		std::vector<Particles::conndata> conns;
		std::vector<std::vector<Particles::conndata>> conns2;
		std::vector<double> bboxs;
		
		int impId, funcId;

		void AllocFrames(uint frames);
		void Clear();
		void Seek(uint f);
		void Update();
		void UpdateMemRange();
	private:
		AnimData(const AnimData&) = delete;
	} static anim;
	static void IncFrame(bool loop);
	static void SetFrame(uint frm);

	static double boundingBox[6];

	static Vec3 colorPallete[256];
	static ushort defColPallete[256];
	static Vec4 _colorPallete[256];
	static byte defColPalleteSz;
	static GLuint colorPalleteTex;
	static bool palleteDirty;

	static void Init(), Clear(), GenTexBufs();
	static void Resize(uint i);
	static void Update(), UpdateBufs(), UpdateColorTex(), UpdateRadBuf(int i = -1);
	static void AddParam(), RmParam(int i);

	static void Serialize(XmlNode* nd);
	static void SerializeVis(XmlNode* nd);
	static void SerializeDM(XmlNode* nd);

	static void Deserialize(XmlNode* nd);
	static void DeserializeVis(XmlNode* nd);
	static void DeserializeDM(XmlNode* nd);

	static GLuint posVao;
	static GLuint posBuffer; //xyz
	static GLuint connBuffer; //uint uint
	static GLuint colIdBuffer; //byte
	static GLuint radBuffer; //float
	static GLuint posTexBuffer, connTexBuffer, colorIdTexBuffer, radTexBuffer;
};