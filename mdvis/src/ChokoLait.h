#pragma once

/*
forked from ChokoLait -- Chokomancarr 2018
*/

#include "Engine.h"

typedef void(*emptyCallbackFunc)(void);
typedef bool(*dropFileFunc)(int, const char**);

class ChokoLait {
public:
	ChokoLait() {
		if (!initd) {
			_InitVars();
			initd = 1;
		}
	}

	static rCamera mainCamera;

	static std::vector<dropFileFunc> dropFuncs;
	static std::vector<emptyCallbackFunc> focusFuncs;

	static void Init(int scrW, int scrH);

	static bool alive();

	static void Update(emptyCallbackFunc func = 0);
	static void Paint(emptyCallbackFunc rendFunc = 0, emptyCallbackFunc paintFunc = 0);

protected:
	static int initd;

	static GLFWwindow* window;

	static void _InitVars();

	static void MouseGL(GLFWwindow* window, int button, int state, int mods);
	static void MouseScrGL(GLFWwindow* window, double xoff, double yoff);
	static void MouseEnterGL(GLFWwindow* window, int e);
	static void MotionGL(GLFWwindow* window, double x, double y);
	static void ReshapeGL(GLFWwindow* window, int w, int h);
	static void DropGL(GLFWwindow* window, int n, const char** fs);
	static void FocusGL(GLFWwindow* window, int focus);
};