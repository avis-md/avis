#pragma once
#include "Engine.h"
#include "Particles.h"

class ParMenu {
public:
	struct ListList {
		ListList(uint a, uint b, uint c) : ResLId(a), ResId(b), ParId(c) {}
		uint ResLId, ResId, ParId;
	};

	static int activeMenu;
	static const string menuNames[4];
	static bool expanded;
	static float expandPos;

	static const uint listMaxItems;
	static std::vector<ListList> listList;

	static uint listListSz, listActive;

	static Font* font;

	static void InitList();

	static void Draw(), Draw_List();
};