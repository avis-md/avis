#pragma once
#include "Engine.h"

//shorthands
Vec4 black(float f = 1);
Vec4 red(float f = 1, float i = 1), green(float f = 1, float i = 1), blue(float f = 1, float i = 1), cyan(float f = 1, float i = 1), yellow(float f = 1, float i = 1), white(float f = 1, float i = 1);

class Color {
public:
	static void Init();

	static GLuint pickerProgH, pickerProgSV;

	static void Rgb2Hsv(byte r, byte g, byte b, float& h, float& s, float& v), Hsv2Rgb(float h, float s, float v, byte& r, byte& g, byte& b);
	static Vec3 Rgb2Hsv(Vec4 col);
	static string Col2Hex(Vec4 col), Col2Hex(byte* bs);
	static void DrawPicker(bool hasA = true);
	static Vec4 HueBaseCol(float hue);

	static void DrawSV(float x, float y, float w, float h, float hue);
	static void DrawH(float x, float y, float w, float h);
};