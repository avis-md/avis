#include "icons.h"

Texture* Icons::expand, *Icons::collapse, *Icons::python, *Icons::play, *Icons::playall;

void Icons::Init() {
	expand = new Texture(IO::path + "/res/expand.png", false, TEX_FILTER_POINT);
	collapse = new Texture(IO::path + "/res/collapse.png", false, TEX_FILTER_POINT);
	python = new Texture(IO::path + "/res/python.png");
	play = new Texture(IO::path + "/res/play.png");
	playall = new Texture(IO::path + "/res/play2.png");
}