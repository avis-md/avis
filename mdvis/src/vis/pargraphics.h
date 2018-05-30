#pragma once
#include "ChokoLait.h"
#include "md/Particles.h"
#include "vis/system.h"

enum class DRAW_TYPE {
	
};

class ParGraphics {
public:
	static Texture* refl;
	static float reflStr, reflStrDecay, rimOff, rimStr;

	static GLuint reflProg, parProg, parConProg;
	static GLint reflProgLocs[11], parProgLocs[7], parConProgLocs[7];

	static GLuint selHlProg, colProg;
	static GLint selHlProgLocs[4], colProgLocs[5];

	static std::vector<uint> hlIds;
	static std::vector<std::pair<uint, std::pair<uint, VIS_DRAW_MODE>>> drawLists, drawListsB;

	static Vec3 rotCenter;
	static uint rotCenterTrackId;
	static float rotW, rotZ;
	static float rotScale;

	static float zoomFade;

	static bool dragging;
	static Vec3 scrX, scrY;

	static bool animate, seek;

	static GLuint emptyVao;

	static void Init(), UpdateDrawLists();

	static void Update();

	static void Rerender();

	//colPallete to gbuffer
	static void Recolor();

	//gbuffers to screen
	static void Reblit();

	static void DrawMenu();

protected:
	static void BlitSky();
	static void BlitHl();
};