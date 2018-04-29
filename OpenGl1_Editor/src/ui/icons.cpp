#include "icons.h"

Texture* Icons::expand, *Icons::collapse, *Icons::python;

void Icons::Init() {
	expand = new Texture(IO::path + "/res/expand.png");
	collapse = new Texture(IO::path + "/res/collapse.png");
	python = new Texture(IO::path + "/res/python.png");
}