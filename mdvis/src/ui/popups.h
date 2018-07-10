#pragma once
#include "Engine.h"

enum class POPUP_TYPE : byte {
    NONE,
	MENU,
    DRAWMODE,
	COLORPICK
};

struct MenuItem {
	typedef void(*CBK)();

	GLuint icon = 0;
	string label;
	std::vector<MenuItem> child;
	CBK callback;

	void Set(Texture* tex, const string& str, CBK cb) {
		icon = tex? tex->pointer : 0;
		label = str;
		callback = cb;
	}
};

class Popups {
public:
    static POPUP_TYPE type;
    static Vec2 pos;
    static void* data;

    static void Draw(), DrawMenu();
	static bool DoDrawMenu(std::vector<MenuItem>* mn, float x, float y);
};