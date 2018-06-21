#pragma once
#include "Engine.h"

/*! Mouse, keyboard, and touch input
[av] */
class Input {
public:
	static Vec2 mousePos, mousePosRelative, mouseDelta, mouseDownPos;
	static bool mouse0, mouse1, mouse2;
	static long long mouseT;
	static bool dbclick;
	static float mouseScroll;
	static byte mouse0State, mouse1State, mouse2State;
	static string inputString;
	static void UpdateMouseNKeyboard(bool* src = nullptr);

	static bool KeyDown(InputKey key), KeyHold(InputKey key), KeyUp(InputKey key);

	Vec2 _mousePos, _mousePosRelative, _mouseDelta, _mouseDownPos;
	bool _mouse0, _mouse1, _mouse2;
	bool _keyStatuses[325];

	static void PreLoop();

	friend class ChokoLait;
	friend class Engine;
protected:
	static void RegisterCallbacks(), TextCallback(GLFWwindow*, uint);
	static bool keyStatusOld[325], keyStatusNew[325];
private:
	static Vec2 mousePosOld;
	static string _inputString;
	static float _mouseScroll;
	//Input(Input const &); //deliberately not defined
	//Input& operator= (Input const &);
};