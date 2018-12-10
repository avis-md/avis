#pragma once
#include "Engine.h"
#include "popups.h"
#include <functional>

//UI with default style settings

class UI2 {
public:
	typedef void(*filecallback) (std::vector<std::string>);

	static void Init();

	static float sepw;

	static void LabelMul(float x, float y, float sz, const std::string& s);
	static std::string EditText(float x, float y, uint w, const std::string& title, const std::string& val, bool enabled = true, Vec4 col = white(1, 0.5f));
	static std::string EditPass(float x, float y, uint w, const std::string& title, const std::string& val, bool enabled = true, Vec4 col = white(1, 0.5f));
	static float Slider(float x, float y, float w, const std::string& title, float a, float b, float t);
	static float Slider(float x, float y, float w, const std::string& title, float a, float b, float t, const std::string& lbl);
	static float Slider(float x, float y, float w, float a, float b, float t);
	static void Color(float x, float y, float w, const std::string& title, Vec4& col);
	static void File(float x, float y, float w, const std::string& title, const std::string& fl, filecallback func);
	static MOUSE_STATUS Button2(float x, float y, float w, const std::string& s, const Texture& tex, Vec4 col = white(1, 0.4f), Vec4 col2 = white());
	static void Dropdown(float x, float y, float w, const std::string& title, const Popups::DropdownItem& data);
	static void Dropdown(float x, float y, float w, const Popups::DropdownItem& data, std::function<void()> func = nullptr, std::string label = "\1", Vec4 col = white());
	static void Toggle(float x, float y, float w, const std::string& title, bool& val);
	static void Switch(float x, float y, float w, const std::string& title, int c, std::string* nms, int& i);
	static void Bezier(Vec2 p1, Vec2 t1, Vec2 t2, Vec2 p2, Vec4 col, float thick = 1, int reso = 20);
	static float Scroll(float x, float y, float h, float t, float tot, float fill);

	static Vec3 EditVec(float x, float y, float w, const std::string& title, Vec3 v, bool ena);

	PROGDEF_H(bezierProg, 5)
};