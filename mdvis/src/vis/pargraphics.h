#pragma once
#include "ChokoLait.h"
#include "md/particles.h"
#include "vis/system.h"

class ParGraphics {
public:
	static Texture bg, splash, logo;
	static GLuint refl, reflE;
	static float reflStr, reflStrDecay, reflStrDecayOff, specStr;
	static bool fogUseBgCol;
	static Vec4 bgCol, fogCol;

	static bool showbbox;
	
	static int reflId, _reflId;
	static std::vector<std::string> reflNms;
	static Popups::DropdownItem reflItms;

	static bool useGradCol;
	static uint gradColParam;
	static Vec4 gradCols[3];
	static bool useConCol, useConGradCol;
	static Vec4 conCol;

	static float radScl;

	enum class ORIENT : uint {
		NONE,
		STRETCH,
		VECTOR
	};
	static ORIENT orientType;
	static float orientStr;
	static uint orientParam[3];
	
	PROGDEF_H(reflProg, 15);
	PROGDEF_H(reflCProg, 10);
	PROGDEF_H(parProg, 20);
	PROGDEF_H(parConProg, 20);
	PROGDEF_H(parConLineProg, 20);
	PROGDEF_H(selHlProg, 5);
	PROGDEF_H(colProg, 10);

	static std::vector<uint> hlIds;
	static std::vector<std::pair<uint, std::pair<uint, byte>>> drawLists, drawListsB;

	static uint usePBR;
	static std::string _usePBRNms[3];
	static const Popups::DropdownItem _usePBRItems;

	static Vec3 rotCenter;
	static uint rotCenterTrackId;
	static float rotW, rotZ;
	static float rotWs, rotZs;
	static float rotScale;

	static bool autoRot;

	static Mesh* arrowMesh;

	static enum class CLIPPING {
		NONE,
		PLANE,
		CUBE
	} clippingType, _clippingType;
	static struct ClipPlane {
		Vec3 center;
		Vec3 norm = Vec3(1, 0, 0);
		float size = 0.1f;
	} clipPlane;
	static struct ClipCube {
		Vec3 center;
		Vec3 size = Vec3(1);
	} clipCube;
	static GLuint clipUbo;
	static Vec4 clippingPlanes[6];

	static float zoomFade;

	static bool dragging;
	static byte dragMode;
	static Vec3 scrX, scrY, scrZ;

	static bool animate, seek;
	static float animOff;
	static int animTarFps;
	static bool tfboDirty;

	static Mat4x4 lastMV, lastP, lastMVP;

	class Eff {
	public:
		static bool expanded;
		static bool showSSAO, showGlow;
		static bool useSSAO, useGlow;

		static float ssaoRad, ssaoStr, ssaoBlur;
		static int ssaoSamples;

		static float glowThres, glowRad, glowStr;

		static void Apply();
		static float DrawMenu(float off);

		static void Serialize(XmlNode* nd);

		static void Deserialize(XmlNode* nd);
	};

	static void Init(), InitClippingMesh();
	static void UpdateDrawLists();
	static void OnLoadConfig();

	static void FillRad(byte* rads);

	static void Update();
	static void UpdateClipping();

	static void Rerender(Vec3 _cpos, Vec3 _cfwd, float _w, float _h);

	static void Reblit();

	static void DrawOverlay();

	static void DrawColMenu();
	static void DrawMenu();
	static void DrawPopupDM();

	static void Serialize(XmlNode* nd);
	static void SerializeCol(XmlNode* nd);

	static void Deserialize(XmlNode* nd);
	static void DeserializeCol(XmlNode* nd);

protected:
	static void BlitSky();
	static void BlitHl();
};