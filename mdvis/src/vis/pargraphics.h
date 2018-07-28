#pragma once
#include "ChokoLait.h"
#include "md/Particles.h"
#include "vis/system.h"

class ParGraphics {
public:
	static Texture* bg, *splash, *logo;
	static GLuint refl, reflE;
	static float reflStr, reflStrDecay, specStr;
	static Vec4 bgCol;
	
	static int reflId, _reflId;
	static std::vector<string> reflNms;
	static Popups::DropdownItem reflItms;

	static bool useGradCol;
	
	static GLuint reflProg, reflCProg, parProg, parConProg, parConLineProg;
	static GLint reflProgLocs[15], reflCProgLocs[10], parProgLocs[10], parConProgLocs[7], parConLineProgLocs[5];

	static GLuint selHlProg, colProg;
	static GLint selHlProgLocs[4], colProgLocs[6];

	static std::vector<uint> hlIds, selIds;
	static std::vector<std::pair<uint, std::pair<uint, byte>>> drawLists, drawListsB;

	static uint usePBR;
	static const string _usePBRNms[3];
	static const Popups::DropdownItem _usePBRItems;

	static Vec3 rotCenter;
	static uint rotCenterTrackId;
	static float rotW, rotZ;
	static float rotScale;

	static float zoomFade;

	static bool dragging;
	static Vec3 scrX, scrY;

	static bool animate, seek;
	static bool tfboDirty;

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
	};

	static void Init(), UpdateDrawLists();

	static void FillRad(byte* rads);

	static void Update();

	static void Rerender(Vec3 _cpos, Vec3 _cfwd, float _w, float _h);

	//colPallete to gbuffer
	static void Recolor();

	//gbuffers to screen
	static void Reblit();

	static void DrawMenu();
	static void DrawPopupDM();

protected:
	static void BlitSky();
	static void BlitHl();
};