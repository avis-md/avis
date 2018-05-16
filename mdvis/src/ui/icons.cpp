#include "icons.h"

Texture* Icons::expand, *Icons::collapse, *Icons::python, *Icons::play, *Icons::playall,
*Icons::left, *Icons::right, *Icons::cross, *Icons::visible, *Icons::hidden, *Icons::circle,
*Icons::select, *Icons::deselect, *Icons::flipselect,
*Icons::toolRot, *Icons::toolMov, *Icons::toolSel,
*Icons::refresh, *Icons::checkbox;

void Icons::Init() {
	expand = new Texture(IO::path + "/res/expand.png", false, TEX_FILTER_POINT);
	collapse = new Texture(IO::path + "/res/collapse.png", false, TEX_FILTER_POINT);
	python = new Texture(IO::path + "/res/python.png", false);
	play = new Texture(IO::path + "/res/play.png", false);
	playall = new Texture(IO::path + "/res/play2.png", false);
	left = new Texture(IO::path + "/res/left.png", false);
	right = new Texture(IO::path + "/res/right.png", false);
	cross = new Texture(IO::path + "/res/cross.png", false);
	visible = new Texture(IO::path + "/res/visible.png", false);
	hidden = new Texture(IO::path + "/res/hidden.png", false);
	circle = new Texture(IO::path + "/res/circle.png", false);
	select = new Texture(IO::path + "/res/select.png", false, TEX_FILTER_POINT);
	deselect = new Texture(IO::path + "/res/deselect.png", false, TEX_FILTER_POINT);
	flipselect = new Texture(IO::path + "/res/flipselect.png", false, TEX_FILTER_POINT);
	toolRot = new Texture(IO::path + "/res/tool_rot.png", false, TEX_FILTER_POINT);
	toolMov = new Texture(IO::path + "/res/tool_mov.png", false, TEX_FILTER_POINT);
	toolSel = new Texture(IO::path + "/res/tool_sel.png", false, TEX_FILTER_POINT);
	refresh = new Texture(IO::path + "/res/refresh.png", false);
	checkbox = new Texture(IO::path + "/res/checkbox.png", false);
}