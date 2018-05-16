#pragma once
#include "Engine.h"

class Icons {
public:
	static void Init();

	static Texture* expand, *collapse, *python, *play, *playall,
		*left, *right, *cross, *visible, *hidden, *circle,
		*select, *deselect, *flipselect,
		*toolRot, *toolMov, *toolSel,
		*refresh, *checkbox;
};