#pragma once
#include "Engine.h"

namespace std
{
    template<typename T, size_t N>
    struct hash<array<T, N> >
    {
        typedef array<T, N> argument_type;
        typedef size_t result_type;

        result_type operator()(const argument_type& a) const
        {
            hash<T> hasher;
            result_type h = 0;
            for (result_type i = 0; i < N; ++i)
            {
                h = h * 31 + hasher(a[i]);
            }
            return h;
        }
    };
}

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
	static float GetLabelW(float s, std::string str, Font* f = font);
	static void Label(float x, float y, float s, std::string str, Vec4 col = black(), float maxw = -1, Font* fnt = font);
	static void Label(float x, float y, float s, const char* str, uint sz, Vec4 col = black(), float maxw = -1, Font* fnt = font);

	//Draws an editable text box. EditText does not work on recursive functions.
	static std::string EditText(float x, float y, float w, float h, float s, Vec4 bcol, const std::string& str, bool delayed = false, Vec4 fcol = black(), bool* changed = nullptr, std::string str2 = "", Font* fnt = font, Vec4 hcol = Vec4(0, 120.f / 255, 215.f / 255, 1), Vec4 acol = white(), bool ser = true);
	static std::string EditTextPass(float x, float y, float w, float h, float s, Vec4 bcol, const std::string& str, char repl, bool delayed = false, Vec4 fcol = black(), bool* changed = nullptr, Font* fnt = font, Vec4 hcol = Vec4(0, 120.f / 255, 215.f / 255, 1), Vec4 acol = white(), bool ser = true);

	static std::unordered_map<size_t, Vec2> scrollWs;
	static Vec2* currentScroll;
	static float currentScrollW0;

	static float BeginScroll(float x, float y, float w, float h, float off = -1);
	static void EndScroll(float off);

	static bool _isDrawingLoop;
	typedef std::array<uintptr_t, UI_MAX_EDIT_TEXT_FRAMES> editframes;
	static editframes _activeEditText, _lastEditText, _editingEditText;
	static std::unordered_map<editframes, ushort> _editTextIds;
	static ushort _activeEditTextId, _editingEditTextId;

	static void GetEditTextId();

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

	static Font* font, *font2;

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
	
	PROGDEF_H(quadProgC, 5)
	PROGDEF_H(quadProgT, 5)
};