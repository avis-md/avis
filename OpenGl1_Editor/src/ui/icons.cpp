#include "icons.h"

Texture* Icons::expand, *Icons::collapse, *Icons::python, *Icons::play, *Icons::playall,
*Icons::left, *Icons::right, *Icons::cross;

void Icons::Init() {
	expand = new Texture(IO::path + "/res/expand.png", false, TEX_FILTER_POINT);
	collapse = new Texture(IO::path + "/res/collapse.png", false, TEX_FILTER_POINT);
	python = new Texture(IO::path + "/res/python.png");
	play = new Texture(IO::path + "/res/play.png");
	playall = new Texture(IO::path + "/res/play2.png");
	left = new Texture(IO::path + "/res/left.png");
	right = new Texture(IO::path + "/res/right.png");
	cross = new Texture(IO::path + "/res/cross.png");
}