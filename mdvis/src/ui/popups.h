#pragma once
#include "Engine.h"

enum class POPUP_TYPE : byte {
    NONE,
    DRAWMODE
};

class Popups {
public:
    static POPUP_TYPE type;
    static Vec2 pos;
    static void* data;

    static void Draw();
};