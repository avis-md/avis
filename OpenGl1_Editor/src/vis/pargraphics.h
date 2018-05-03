#pragma once
#include "ChokoLait.h"
#include "md/Particles.h"

class ParGraphics {
public:
	static Texture* refl;

	static GLuint reflProg, parProg, parConProg;
	static GLint reflProgLocs[8], parProgLocs[5], parConProgLocs[6];

	static GLuint emptyVao;

	static void Init();

	static void Rerender();

	//colPallete to gbuffer
	static void Recolor();

	//gbuffers to screen
	static void Reblit();

protected:
	static void BlitSky();
};