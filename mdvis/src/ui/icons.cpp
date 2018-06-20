#include "icons.h"

Texture* Icons::expand, *Icons::collapse, *Icons::play, *Icons::playall,
*Icons::left, *Icons::right, *Icons::up, *Icons::down, *Icons::cross, *Icons::visible, *Icons::hidden, *Icons::circle,
*Icons::select, *Icons::deselect, *Icons::flipselect,
*Icons::toolRot, *Icons::toolMov, *Icons::toolSel,
*Icons::refresh, *Icons::checkbox, *Icons::browse,
*Icons::zoomIn, *Icons::zoomOut,
*Icons::lang_c, *Icons::lang_py, *Icons::lang_ft, *Icons::lightning,
*Icons::icon_anl, *Icons::log, *Icons::openfile,
*Icons::dm_none, *Icons::dm_point, *Icons::dm_ball, *Icons::dm_vdw, *Icons::dm_line, *Icons::dm_stick, *Icons::dm_lineball, *Icons::dm_stickball;

#define TEXN(nm) new Texture(IO::path + "/res/" #nm ".png", false)
#define TEX(nm) nm = new Texture(IO::path + "/res/" #nm ".png", false)
#define TEXNP(nm) new Texture(IO::path + "/res/" #nm ".png", false, TEX_FILTER_POINT)
#define TEXP(nm) nm = new Texture(IO::path + "/res/" #nm ".png", false, TEX_FILTER_POINT)

void Icons::Init() {
	TEXP(expand);
	TEXP(collapse);
	TEX(play);
	playall = TEXN(play2);
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
	zoomIn = TEXN(zoomin);
	zoomOut = TEXN(zoomout);
	TEX(lang_c);
	lang_py = TEXN(lang_python);
	lang_ft = TEXN(lang_fortran);
	TEX(lightning);
	TEX(icon_anl);
	TEX(log);
	TEX(openfile);
	TEX(dm_none);
	TEX(dm_point);
	TEX(dm_ball);
	TEX(dm_vdw);
	TEX(dm_line);
	TEX(dm_stick);
	TEX(dm_lineball);
	TEX(dm_stickball);
}

Texture* Icons::OfDM(byte b) {
	switch (b) {
		case 0x01: return dm_point;
		case 0x02: return dm_ball;
		case 0x03: return dm_vdw;
		case 0x10: case 0x11: return dm_line;
		case 0x12: return dm_lineball;
		case 0x20: case 0x21: return dm_stick;
		case 0x22: return dm_stickball;
		default: return nullptr;
	}
}