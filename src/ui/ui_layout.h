#pragma once

#define _ui_layout_begin(x, y, w) {\
	static float _h = 2;\
	{\
		const float __x = x, __y = y, __w = w;\
		float _x0 = __x, _x = _x0 + 1;\
		float _off0 = __y, _off = _off0 + 1;\
		float _w = __w - 2;

#define ui_layout_begin(x, y, w) _ui_layout_begin(x, y, w)

#define ui_layout_beginc(x, y, w, col)\
	_ui_layout_begin(x, y, w)\
	UI::Quad(_x0, _off0, _w + 2, _h, col);

#define ui_layout_end\
		_h = _off - _off0 + 1;\
	}\
}

#define ui_layout_push(x, y, w) {\
	ui_layout_begin(_x0 + 1 + x, _off + y, _w - w - x)

#define ui_layout_push(x, y, w, col) {\
	ui_layout_begin(_x0 + 1 + x, _off + y, _w - w - x, col)

#define ui_layout_pop\
		_h = (_h = _off - _off0) + 1;\
	}\
	_off += _h;\
}

#define ui_layout_xyw _x, _off, _w