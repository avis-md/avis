#pragma once
#include "Engine.h"

/*! Mouse, keyboard, and touch input
[av] */
class Input {
public:
	static Vec2 mousePos, mousePosRelative, mouseDelta, mouseDownPos;
	static bool mouse0, mouse1, mouse2;
	static bool _mouse0, _mouse1, _mouse2;
	static long long mouseT;
	static bool dbclick;
	static float mouseScroll;
	static byte mouse0State, mouse1State, mouse2State;
	static byte _mouse0State;
	static std::string inputString;
	static void UpdateMouseNKeyboard(bool* src = nullptr);

	static float scrollScl;

	static bool KeyDown(KEY key), KeyHold(KEY key), KeyUp(KEY key);

	static void PreLoop();
	static bool CheckCopy(const char* loc, size_t len);

	friend class ChokoLait;
	friend class Engine;
protected:
	static void RegisterCallbacks(), TextCallback(GLFWwindow*, uint);
	static bool keyStatusOld[325], keyStatusNew[325];
private:
	static Vec2 mousePosOld;
	static std::string _inputString;
	static float _mouseScroll;
};