#pragma once
#include "Engine.h"

class Icons {
public:
	static void Init();

	static Texture* expand, *collapse, *play, *playall,
		*left, *right, *cross, *visible, *hidden, *circle,
		*select, *deselect, *flipselect,
		*toolRot, *toolMov, *toolSel,
		*refresh, *checkbox, *browse,
		*zoomIn, *zoomOut,
		*lang_c, *lang_py, *lang_ft, *lightning,
		*icon_anl,
		*dm_none,
		*dm_point, *dm_ball, *dm_vdw,
		*dm_line, *dm_stick,
		*dm_lineball, *dm_stickball;
	
	static Texture* OfDM(byte b);
};