#pragma once
#include "Engine.h"

enum class POPUP_TYPE : byte {
    NONE,
	MENU,
    DRAWMODE,
	COLORPICK,
	DROPDOWN,
	RESNM,
	RESID,
	ATOMID,
	SYSMSG
};

struct MenuItem {
	typedef void(*CBK)();

	GLuint icon = 0;
	std::string label;
	std::vector<MenuItem> child;
	CBK callback;

	void Set(Texture* tex, const std::string& str, CBK cb) {
		icon = tex? tex->pointer : 0;
		label = str;
		callback = cb;
	}
};

class Popups {
public:
	struct DropdownItem {
		DropdownItem(uint* a, std::string* b) : target(a), list(b) {}

		uint* target;
		std::string* list;
	};

    static POPUP_TYPE type;
    static Vec2 pos, pos2;
    static void* data;

    static void Draw(), DrawMenu(), DrawDropdown();
	static bool DoDrawMenu(std::vector<MenuItem>* mn, float x, float y);
};