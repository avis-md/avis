#pragma once
#include "Engine.h"
#include <functional>

//UI with default style settings

class UI2 {
public:
	static string EditText(float x, float y, uint w, const string& title, const string& val, bool enabled = true, Vec4 col = white(1, 0.5f));
	static float Slider(float x, float y, uint w, const string& title, float a, float b, float t);
	static float Slider(float x, float y, uint w, const string& title, float a, float b, float t, const string& lbl);
	static void Color(float x, float y, uint w, const string& title, Vec4& col);
	static void File(float x, float y, uint w, const string& title, const string& fl, std::function<void(std::vector<string>)> func);
	static MOUSE_STATUS Button2(float x, float y, float w, const string& s, Texture* tex, Vec4 col = white(1, 0.4f), Vec4 col2 = white());
};