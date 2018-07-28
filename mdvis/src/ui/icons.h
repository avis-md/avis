#pragma once
#include "Engine.h"

class Icons {
public:
	static void Init();

	static Texture* expand, *collapse, *play, *playall, *pause,
		*left, *right, *up, *down, *cross, *visible, *hidden, *circle,
		*select, *deselect, *flipselect,
		*toolRot, *toolMov, *toolSel,
		*refresh, *checkbox, *browse, *dropdown2,
		*zoomIn, *zoomOut,
		*lang_c, *lang_py, *lang_ft, *lightning,
		*icon_anl, *log, *newfile, *openfile,
		*dm_none,
		*dm_point, *dm_ball, *dm_vdw,
		*dm_line, *dm_stick,
		*dm_lineball, *dm_stickball,
		*colorwheel,
		*vis_atom, *vis_prot,
		*pro_col, *pro_grad,
		*details;
	
	static Texture* OfDM(byte b);
};