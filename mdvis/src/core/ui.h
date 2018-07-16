#pragma once
#include "Engine.h"

/*! 2D drawing to the screen.
[av] */
class UI {
public:
	static void Texture(float x, float y, float w, float h, ::Texture* texture, DRAWTEX_SCALING scl = DRAWTEX_STRETCH, float miplevel = 0);
	static void Texture(float x, float y, float w, float h, ::Texture* texture, float alpha, DRAWTEX_SCALING scl = DRAWTEX_STRETCH, float miplevel = 0);
	static void Texture(float x, float y, float w, float h, ::Texture* texture, Vec4 tint, DRAWTEX_SCALING scl = DRAWTEX_STRETCH, float miplevel = 0);
	static void Label(float x, float y, float s, string str, Vec4 col = black(), float maxw = -1, Font* fnt = font);
	static void Label(float x, float y, float s, const char* str, uint sz, Vec4 col = black(), float maxw = -1, Font* fnt = font);

	//Draws an editable text box. EditText does not work on recursive functions.
	static string EditText(float x, float y, float w, float h, float s, Vec4 bcol, const string& str, bool delayed = false, Vec4 fcol = black(), bool* changed = nullptr, Font* fnt = font, Vec4 hcol = Vec4(0, 120.0f / 255, 215.0f / 255, 1), Vec4 acol = white(), bool ser = true);
	static string EditTextPass(float x, float y, float w, float h, float s, Vec4 bcol, const string& str, char repl, bool delayed = false, Vec4 fcol = black(), bool* changed = nullptr, Font* fnt = font, Vec4 hcol = Vec4(0, 120.0f / 255, 215.0f / 255, 1), Vec4 acol = white(), bool ser = true);

	static bool CanDraw();

	static bool _isDrawingLoop;
	static void PreLoop();
	static uintptr_t _activeEditText[UI_MAX_EDIT_TEXT_FRAMES], _lastEditText[UI_MAX_EDIT_TEXT_FRAMES], _editingEditText[UI_MAX_EDIT_TEXT_FRAMES];
	static ushort _activeEditTextId, _editingEditTextId;

	static void GetEditTextId();
	static bool IsActiveEditText();
	static bool IsSameId(uintptr_t* left, uintptr_t* right);

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

	static Font* font;

	static bool focused, editingText;
	static uint _editTextCursorPos, _editTextCursorPos2;
	static string _editTextString;
	static float _editTextBlinkTime;

	static Style _defaultStyle;
	static float alpha;

	static void InitVao(), SetVao(uint sz, void* verts, void* uvs = nullptr);
	static uint _vboSz;
	static GLuint _vao, _vboV, _vboU;

	static byte _layer, _layerMax;

	static void Init(), IncLayer();
};