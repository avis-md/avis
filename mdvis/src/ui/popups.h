#pragma once
#include "Engine.h"

enum class POPUP_TYPE : byte {
    NONE,
    DRAWMODE,
	COLORPICK
};

struct MenuItem {
	GLuint icon;
	string label;
	std::vector<MenuItem> child;
};

class Popups {
public:
    static POPUP_TYPE type;
    static Vec2 pos;
    static void* data;

    static void Draw();
};