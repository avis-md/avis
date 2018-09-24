#include "Engine.h"

int Display::width = 256, Display::height = 256;
int Display::actualWidth = 256, Display::actualHeight = 256;
int Display::frameWidth = 256, Display::frameHeight = 256;
glm::mat3 Display::uiMatrix = glm::mat3();
bool Display::uiMatrixIsI = true;

float Display::dpiScl = 1;

NativeWindow* Display::window = nullptr;
CURSORTYPE Display::cursorType;

void Display::Resize(int x, int y, bool maximize) {
	glfwSetWindowSize(window, x, y);
}

void Display::SetCursor(CURSORTYPE type) {
	cursorType = type;
}