#pragma once
#include "Engine.h"

enum CURSORTYPE : byte {
	CURSORTYPE_NORMAL,
	CURSORTYPE_TEXT,
	CURSORTYPE_SELECT
};

/*! Display window properties
[av] */
class Display {
public:
	static int width, height;
	static int actualWidth, actualHeight;
	static int frameWidth, frameHeight;
	static glm::mat3 uiMatrix;
	static bool uiMatrixIsI;

	static float dpiScl;

	static void Resize(int x, int y, bool maximize = false);

	static void SetCursor(CURSORTYPE);

	friend int main(int argc, char **argv);
	//move all functions in here please
	friend void MouseGL(GLFWwindow* window, int button, int state, int mods);
	friend void MotionGL(GLFWwindow* window, double x, double y);
	friend void FocusGL(GLFWwindow* window, int focus);
	friend class PopupSelector;
	friend class ChokoLait;

	static NativeWindow* window;
	static CURSORTYPE cursorType;
};