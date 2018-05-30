#pragma once
#include "Engine.h"

class Icons {
public:
	static void Init();

	static Texture* expand, *collapse, *python, *play, *playall,
		*left, *right, *cross, *visible, *hidden, *circle,
		*select, *deselect, *flipselect,
		*toolRot, *toolMov, *toolSel,
		*refresh, *checkbox,
		*zoomIn, *zoomOut,
		*dm_none,
		*dm_point, *dm_ball, *dm_vdw,
		*dm_line, *dm_stick,
		*dm_lineball, *dm_stickball;
	
	static Texture* OfDM(byte b);
};