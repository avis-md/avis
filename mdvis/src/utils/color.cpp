#include "Engine.h"
#include "ui/popups.h"
#include "ui/icons.h"

GLuint Color::pickerProgH = 0;
GLuint Color::pickerProgSV = 0;

void Color::Init() {
	std::vector<string> s2 = string_split(DefaultResources::GetStr("e_colorPickerSV.txt"), '$');
	Color::pickerProgSV = Shader::FromVF(s2[0], s2[1]);

	s2 = string_split(DefaultResources::GetStr("e_colorPickerH.txt"), '$');
	Color::pickerProgH = Shader::FromVF(s2[0], s2[1]);
}

void Color::Hsv2Rgb(float h, float s, float v, byte& r, byte& g, byte& b) {
	Vec4 cb = HueBaseCol(h);
	Vec4 c(Lerp(Lerp(cb, Vec4(1, 1, 1, 1), 1 - s), Vec4(), 1 - v));
	r = (byte)round(c.r * 255);
	g = (byte)round(c.g * 255);
	b = (byte)round(c.b * 255);
}

void Color::Rgb2Hsv(byte r, byte g, byte b, float& h, float& s, float& v) {
	float R = r * 0.0039216f;
	float G = g * 0.0039216f;
	float B = b * 0.0039216f;

	float mn = min(min(R, G), B);
	float mx = max(max(R, G), B);

	v = mx;
	if (mx > 0) {
		s = (mx - mn) / mx;
		if (mn != mx) {
			if (R == mx) {
				h = (G - B) / (mx - mn);
			}
			else if (G == mx) {
				h = 2.0f + (B - R) / (mx - mn);
			}
			else {
				h = 4.0f + (R - G) / (mx - mn);
			}
			h /= 6;
			if (h < 0) h += 1;
		}
	}
}

Vec3 Color::Rgb2Hsv(Vec4 col) {
	Vec3 o = Vec3();
	Rgb2Hsv((byte)(col.r * 255), (byte)(col.g * 255), (byte)(col.b * 255), o.r, o.g, o.b);
	return o;
}

string Color::Col2Hex(Vec4 col) {
	byte bs[] = { (byte)(col.r * 255), (byte)(col.g * 255), (byte)(col.b * 255), (byte)(col.a * 255) };
	return Col2Hex(bs);
}

string Color::Col2Hex(byte* bs) {
	const char* hx = "0123456789ABCDEF";
	string res = "XXXXXXXX";
	for (byte a = 0; a < 4; a++) {
		auto i = bs[a];
		res[a * 2] = hx[i >> 4];
		res[a * 2 + 1] = hx[i & 15];
	}
	return res;
}

void Color::DrawPicker(bool hasA) {
	auto& cl = *((Vec4*)Popups::data);

	Vec3 hsv = Rgb2Hsv(cl);

	Engine::DrawQuad(Popups::pos.x, Popups::pos.y, 150, 150, black(0.7f));
	Color::DrawSV(Popups::pos.x + 5, Popups::pos.y + 5, 120, 120, hsv.r);
	Vec2 sv(1-hsv.y, hsv.z);
	sv = Engine::DrawSliderFill2D(Popups::pos.x + 5, Popups::pos.y + 5, 120, 120, Vec2(), Vec2(1, 1), sv, white(0), white(0));
	hsv.y = 1-sv.x;
	hsv.z = sv.y;
	UI::Texture(Popups::pos.x + 120 * sv.x, Popups::pos.y + 120 - 120 * sv.y, 10, 10, Icons::circle, (sv.y > 0.5f) ? black() : white());
	UI::Texture(Popups::pos.x + 1 + 120 * sv.x, Popups::pos.y + 121 - 120 * sv.y, 8, 8, Icons::circle, cl);
	Color::DrawH(Popups::pos.x + 132, Popups::pos.y + 5, 15, 120);
	hsv.x = Engine::DrawSliderFillY(Popups::pos.x + 132, Popups::pos.y + 5, 15, 120, 0, 1, hsv.x, white(0), white(0));
	Engine::DrawQuad(Popups::pos.x + 130, Popups::pos.y + 4 + 120 * hsv.x, 19, 2, white());

	if ((Input::mouse0State == 1) && !Engine::Button(Popups::pos.x, Popups::pos.y, 150, 150)) {
		Popups::type = POPUP_TYPE::NONE;
	}

	byte bs[4];
	Hsv2Rgb(hsv.x, hsv.y, hsv.z, bs[0], bs[1], bs[2]);
	bs[3] = 255;

	string hx = Col2Hex(bs);

	hx = UI::EditText(Popups::pos.x + 5, Popups::pos.y + 130, 120, 16, 12, white(1, 0.5f), hx, true, white());

	cl = Vec4(bs[0] / 255.0f, bs[1] / 255.0f, bs[2] / 255.0f, 1);
}

Vec4 Color::HueBaseCol(float hue) {
	hue *= 6;
	Vec4 v;
	v.r = Clamp(abs(hue - 3) - 1.0f, 0.0f, 1.0f);
	v.g = 1 - Clamp(abs(hue - 2) - 1.0f, 0.0f, 1.0f);
	v.b = 1 - Clamp(abs(hue - 4) - 1.0f, 0.0f, 1.0f);
	v.a = 1;
	return v;
}

void Color::DrawSV(float x, float y, float w, float h, float hue) {
	Vec3 quadPoss[4];
	Vec2 quadUvs[4]{ Vec2(0, 1), Vec2(1, 1), Vec2(0, 0), Vec2(1, 0) };
	quadPoss[0].x = x;
	quadPoss[0].y = y;
	quadPoss[1].x = x + w;
	quadPoss[1].y = y;
	quadPoss[2].x = x;
	quadPoss[2].y = y + h;
	quadPoss[3].x = x + w;
	quadPoss[3].y = y + h;
	for (int y = 0; y < 4; y++) {
		quadPoss[y].z = 1;
		quadPoss[y] = Ds(Display::uiMatrix*quadPoss[y]);
	}

	UI::SetVao(4, quadPoss);

	glUseProgram(pickerProgSV);
	Vec4 bc = HueBaseCol(hue);
	GLint baseColLoc = glGetUniformLocation(pickerProgSV, "col");
	glUniform3f(baseColLoc, bc.r, bc.g, bc.b);

	glBindVertexArray(UI::_vao);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, Engine::quadBuffer);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
	glUseProgram(0);
}

void Color::DrawH(float x, float y, float w, float h) {
	Vec3 quadPoss[4];
	Vec2 quadUvs[4]{ Vec2(0, 1), Vec2(1, 1), Vec2(0, 0), Vec2(1, 0) };
	quadPoss[0].x = x;
	quadPoss[0].y = y;
	quadPoss[1].x = x + w;
	quadPoss[1].y = y;
	quadPoss[2].x = x;
	quadPoss[2].y = y + h;
	quadPoss[3].x = x + w;
	quadPoss[3].y = y + h;
	for (int y = 0; y < 4; y++) {
		quadPoss[y].z = 1;
		quadPoss[y] = Ds(Display::uiMatrix*quadPoss[y]);
	}

	UI::SetVao(4, quadPoss);

	glUseProgram(pickerProgH);
	glBindVertexArray(UI::_vao);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, Engine::quadBuffer);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
	glUseProgram(0);
}