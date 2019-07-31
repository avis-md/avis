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