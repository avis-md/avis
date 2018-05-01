#include "Engine.h"

int Display::width = 512;
int Display::height = 512;
glm::mat3 Display::uiMatrix = glm::mat3();
NativeWindow* Display::window = nullptr;

void Display::Resize(int x, int y, bool maximize) {
#ifdef PLATFORM_WIN
	glfwSetWindowSize(window, x, y);
	ShowWindow(GetActiveWindow(), maximize ? SW_MAXIMIZE : SW_NORMAL);
#elif defined(PLATFORM_ADR)
	ANativeWindow_setBuffersGeometry(window, x, y, 0);
#endif
}