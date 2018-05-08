#pragma once
#include "ChokoLait.h"
#include "md/Particles.h"

class ParGraphics {
public:
	static Texture* refl;
	static float reflStr, reflStrDecay, rimOff, rimStr;

	static GLuint reflProg, parProg, parConProg;
	static GLint reflProgLocs[11], parProgLocs[5], parConProgLocs[7];

	static GLuint selHlProg, colProg;
	static GLint selHlProgLocs[4], colProgLocs[5];

	static std::vector<uint> hlIds;
	static std::vector<std::pair<uint, uint>> drawLists, drawListsB;

	static Vec3 rotCenter;
	static float rotW, rotZ;
	static float rotScale;

	static bool dragging;
	static Vec3 scrX, scrY;

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