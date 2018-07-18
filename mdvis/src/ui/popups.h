#pragma once
#include "Engine.h"

enum class POPUP_TYPE : byte {
    NONE,
	MENU,
    DRAWMODE,
	COLORPICK,
	DROPDOWN
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
	struct DropdownItem {
		DropdownItem(uint* a, string* b) : target(a), list(b) {}

		uint* target;
		string* list;
	};

    static POPUP_TYPE type;
    static Vec2 pos, pos2;
    static void* data;

    static void Draw(), DrawMenu(), DrawDropdown();
	static bool DoDrawMenu(std::vector<MenuItem>* mn, float x, float y);
};