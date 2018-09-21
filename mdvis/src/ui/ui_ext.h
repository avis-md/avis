#pragma once
#include "Engine.h"
#include "popups.h"
#include <functional>

//UI with default style settings

class UI2 {
public:
	static void Init();

	static void LabelMul(float x, float y, float sz, const std::string& s);
	static std::string EditText(float x, float y, uint w, const std::string& title, const std::string& val, bool enabled = true, Vec4 col = white(1, 0.5f));
	static float Slider(float x, float y, uint w, const std::string& title, float a, float b, float t);
	static float Slider(float x, float y, uint w, const std::string& title, float a, float b, float t, const std::string& lbl);
	static void Color(float x, float y, uint w, const std::string& title, Vec4& col);
	static void File(float x, float y, uint w, const std::string& title, const std::string& fl, std::function<void(std::vector<std::string>)> func);
	static MOUSE_STATUS Button2(float x, float y, float w, const std::string& s, Texture* tex, Vec4 col = white(1, 0.4f), Vec4 col2 = white());
	static void Dropdown(float x, float y, float w, const std::string& title, const Popups::DropdownItem& data);
	static void Toggle(float x, float y, float w, const std::string& title, bool& val);
	static void Switch(float x, float y, float w, const std::string& title, int c, std::string* nms, int& i);
	static void Bezier(Vec2 p1, Vec2 t1, Vec2 t2, Vec2 p2, Vec4 col, int reso = 20);

	PROGDEF_H(bezierProg, 5)
};