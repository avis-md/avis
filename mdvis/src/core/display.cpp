#include "Engine.h"

int Display::width = 512;
int Display::height = 512;
int Display::actualWidth = 512;
int Display::actualHeight = 512;
glm::mat3 Display::uiMatrix = glm::mat3();
bool Display::uiMatrixIsI = true;
NativeWindow* Display::window = nullptr;
CURSORTYPE Display::cursorType;

void Display::Resize(int x, int y, bool maximize) {
#if defined(PLATFORM_ADR)
	ANativeWindow_setBuffersGeometry(window, x, y, 0);
#else
	glfwSetWindowSize(window, x, y);
#ifdef PLATFORM_WIN
	ShowWindow(GetActiveWindow(), maximize ? SW_MAXIMIZE : SW_NORMAL);
#endif
#endif
}

void Display::SetCursor(CURSORTYPE type) {
	cursorType = type;
}