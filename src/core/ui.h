// Copyright (C) 2019 Pua Kai
// 
// This file is part of AViS.
// 
// AViS is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// AViS is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with AViS.  If not, see <http://www.gnu.org/licenses/>.

#pragma once
#include "Engine.h"
#include "utils/uniquecaller.h"

/*! 2D drawing to the screen.
[av] */
class UI {
public:
	static void Init(), IncLayer();

	static void PreLoop();

	static void Rotate(float a, Vec2 point);
	static void ResetMatrix();

	static void Quad(float x, float y, float w, float h, Vec4 col);
	static void Quad(float x, float y, float w, float h, GLuint tex, Vec4 col = white(), int mip = -1, Vec2 uv0 = Vec2(0, 1), Vec2 uv1 = Vec2(1, 1), Vec2 uv2 = Vec2(0, 0), Vec2 uv3 = Vec2(1, 0));
	static void Texture(float x, float y, float w, float h, const ::Texture& texture, DRAWTEX_SCALING scl = DRAWTEX_STRETCH, float miplevel = 0);
	static void Texture(float x, float y, float w, float h, const ::Texture& texture, float alpha, DRAWTEX_SCALING scl = DRAWTEX_STRETCH, float miplevel = 0);
	static void Texture(float x, float y, float w, float h, const ::Texture& texture, Vec4 tint, DRAWTEX_SCALING scl = DRAWTEX_STRETCH, float miplevel = 0);
	static float GetLabelW(float s, std::string str, Font* f = &font);
	static void Label(float x, float y, float s, const std::string& str, Vec4 col = black(), bool shad = false, float maxw = -1, Font* fnt = &font);
	static void Label(float x, float y, float s, const char* str, uint sz, Vec4 col = black(), bool shad = false, float maxw = -1, Font* fnt = &font);

	//Draws an editable text box. EditText does not work on recursive functions.
	static std::string EditText(float x, float y, float w, float h, float s, Vec4 bcol, const std::string& str, bool delayed = false, Vec4 fcol = black(), bool* changed = nullptr, std::string str2 = "", Font* fnt = &font, Vec4 hcol = Vec4(0, 120.f / 255, 215.f / 255, 1), Vec4 acol = white(), bool ser = true);
	static std::string EditTextPass(float x, float y, float w, float h, float s, Vec4 bcol, const std::string& str, char repl, bool delayed = false, Vec4 fcol = black(), bool* changed = nullptr, Font* fnt = &font, Vec4 hcol = Vec4(0, 120.f / 255, 215.f / 255, 1), Vec4 acol = white(), bool ser = true);

	static std::unordered_map<size_t, Vec2> scrollWs;
	static Vec2* currentScroll;
	static float currentScrollW0;

	static float BeginScroll(float x, float y, float w, float h, float off = -1);
	static void EndScroll(float off);

	static bool _isDrawingLoop;

	static UniqueCallerList _editTextCallee;

	struct StyleColor {
		Vec4 backColor, fontColor;
		//Texture* backTex;

		void Set(const Vec4 vb, const Vec4 vf) {
			backColor = vb;
			fontColor = vf;
		}
	};
	struct Style {
		StyleColor normal, mouseover, highlight, press;
		int fontSize;
	};

	static glm::mat3 matrix;
	static bool matrixIsI;

	static Font font, font2;

	static bool focused, editingText;
	static uint _editTextCursorPos, _editTextCursorPos2;
	static std::string _editTextString;
	static float _editTextBlinkTime;
	static float _editTextHoff;

	static Style _defaultStyle;
	static float alpha;

	static void InitVao(), SetVao(uint sz, void* verts, void* uvs = nullptr);
	static uint _vboSz;
	static GLuint _vao, _vboV, _vboU, _tvbo;

	static byte _layer, _layerMax;
	static bool ignoreLayers;
	
	static Shader quadProgC, quadProgT;
};