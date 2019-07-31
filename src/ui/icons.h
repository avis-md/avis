// Copyright (C) 2019 Pua Kai
// 
// This file is part of AViS.
// 
// AViS is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// AViS is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with AViS.  If not, see <http://www.gnu.org/licenses/>.

#pragma once
#include "Engine.h"

class Icons {
public:
	static void Init();

	static Texture expand, collapse, play, playall, pause,
		left, right, up, down, cross, visible, hidden, circle,
		select, deselect, flipselect,
		toolRot, toolMov, toolSel,
		refresh, checkbox, browse, dropdown2,
		zoomIn, zoomOut,
		lang_c, lang_py, lang_ft, lightning,
		icon_anl, log, newfile, openfile,
		dm_none,
		dm_point, dm_ball, dm_vdw,
		dm_line, dm_stick,
		dm_lineball, dm_stickball,
		colorwheel,
		vis_atom, vis_prot,
		pro_col, pro_grad,
		details, compile, exec, tick, help;
	
	static const Texture& OfDM(byte b);
};