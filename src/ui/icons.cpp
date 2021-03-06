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

#include "icons.h"
#include "res/resdata.h"

Texture Icons::expand, Icons::collapse, Icons::play, Icons::playall, Icons::pause,
Icons::left, Icons::right, Icons::up, Icons::down, Icons::cross, Icons::visible, Icons::hidden, Icons::circle,
Icons::select, Icons::deselect, Icons::flipselect,
Icons::toolRot, Icons::toolMov, Icons::toolSel,
Icons::refresh, Icons::checkbox, Icons::browse, Icons::dropdown2,
Icons::zoomIn, Icons::zoomOut,
Icons::lang_c, Icons::lang_py, Icons::lang_ft, Icons::lightning,
Icons::icon_anl, Icons::log, Icons::newfile, Icons::openfile,
Icons::dm_none, Icons::dm_point, Icons::dm_ball, Icons::dm_vdw, Icons::dm_line, Icons::dm_stick, Icons::dm_lineball, Icons::dm_stickball,
Icons::colorwheel,
Icons::vis_atom, Icons::vis_prot,
Icons::pro_col, Icons::pro_grad,
Icons::details, Icons::compile, Icons::exec, Icons::tick, Icons::help;

#define TEXN(nm) Texture(res::nm ## _png, res::nm ## _png_sz)
#define TEX(nm) nm = Texture(res::nm ## _png, res::nm ## _png_sz)
#define TEXNP(nm) Texture(res::nm ## _png, res::nm ## _png_sz, TEX_FILTER_POINT)
#define TEXP(nm) nm = Texture(res::nm ## _png, res::nm ## _png_sz, TEX_FILTER_POINT)

void Icons::Init() {
	TEXP(expand);
	TEXP(collapse);
	TEX(play);
	playall = TEXN(play2);
	TEX(pause);
	TEX(left);
	TEX(right);
	TEX(up);
	TEX(down);
	TEX(cross);
	TEX(visible);
	TEX(hidden);
	TEX(circle);
	TEXP(select);
	TEXP(deselect);
	TEXP(flipselect);
	toolRot = TEXNP(tool_rot);
	toolMov = TEXNP(tool_mov);
	toolSel = TEXNP(tool_sel);
	TEX(refresh);
	TEX(checkbox);
	TEX(browse);
	TEX(dropdown2);
	zoomIn = TEXN(zoomin);
	zoomOut = TEXN(zoomout);
	TEX(lang_c);
	lang_py = TEXN(lang_python);
	lang_ft = TEXN(lang_fortran);
	TEX(lightning);
	TEX(icon_anl);
	TEX(log);
	TEX(newfile);
	TEX(openfile);
	TEX(dm_none);
	TEX(dm_point);
	TEX(dm_ball);
	TEX(dm_vdw);
	TEX(dm_line);
	TEX(dm_stick);
	TEX(dm_lineball);
	TEX(dm_stickball);
	TEX(colorwheel);
	TEX(vis_atom);
	TEX(vis_prot);
	TEX(pro_col);
	TEX(pro_grad);
	TEX(details);
	TEX(compile);
	TEX(exec);
	TEX(tick);
	TEX(help);
}

const Texture& Icons::OfDM(byte b) {
	switch (b) {
		case 0x01: return dm_point;
		case 0x02: return dm_ball;
		case 0x03: return dm_vdw;
		case 0x10: case 0x11: return dm_line;
		case 0x12: return dm_lineball;
		case 0x20: case 0x21: return dm_stick;
		case 0x22: return dm_stickball;
		default: return dm_none;
	}
}