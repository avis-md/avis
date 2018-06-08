#include "Engine.h"

void fopen_s(FILE** f, const char* c, const char* m) {
	*f = fopen(c, m);
}
void _putenv_s(string nm, const char* loc) {
	string s = ((nm) + "=" + string(loc));
	putenv(&s[0]);
}

Vec4 black(float f) { return Vec4(0, 0, 0, f); }
Vec4 red(float f, float i) { return Vec4(i, 0, 0, f); }
Vec4 green(float f, float i) { return Vec4(0, i, 0, f); }
Vec4 blue(float f, float i) { return Vec4(0, 0, i, f); }
Vec4 cyan(float f, float i) { return Vec4(i*0.09f, i*0.706f, i, f); }
Vec4 yellow(float f, float i) { return Vec4(i, i, 0, f); }
Vec4 white(float f, float i) { return Vec4(i, i, i, f); }