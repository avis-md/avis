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

#include "anweb.h"
#include "arrayview.h"
#include "ui/ui_ext.h"
#include "ui/icons.h"

bool ArrayView::show = false;
void* ArrayView::data = nullptr;
std::string ArrayView::scrNm, ArrayView::varNm;
char ArrayView::type;
std::vector<int*> ArrayView::dims;
int ArrayView::windowSize = 200;

void ArrayView::Draw() {
	if (data) {
		if (UI2::Button2(80, Display::height - 17.f, 40, "[...]", Icons::circle, white(0)) == MOUSE_RELEASE) {
			show = !show;
		}
		if (show) {
			//UI::IncLayer();
			float y = Display::height - 18.f - windowSize;
			UI::Quad(0, y, (float)Display::width, (float)windowSize, white(0.95f, 0.1f));
			y++;
			UI::Label(2, y, 12, "Contents of: " + scrNm + " > " + varNm, white());
			if (Engine::Button(Display::width - 35.f, y, 16, 16, Icons::collapse) == MOUSE_RELEASE) {
				show = false;
			}
			if (Engine::Button(Display::width - 18.f, y, 16, 16, Icons::cross) == MOUSE_RELEASE) {
				data = nullptr;
				return;
			}
			y += 19;

			auto dim = dims.size();
			int ts = 1;
			std::vector<int> szs(dim);
			for (size_t a = 0; a < dim; a++) {
				ts *= (szs[a] = *dims[dim-a-1]);
			}
			if (!ts) {
				UI::Label(5, y, 12, "Array is empty", white(0.8f));
			}
			else {
				const auto hmax = Display::height - y - 18.f;
				Engine::BeginStencil(0, y, (float)Display::width, hmax);
				int dw = 71;
				int nw = (int)std::floor((Display::width - 60) / dw);
				int nh = (ts + nw - 1) / nw;
				dw = (Display::width - 60) / nw;
				int ix = 0;
				static float y0 = 0;
				y0 = UI2::Scroll(Display::width - 8.f, y, hmax, y0, nh * 17.f, hmax);
				if (Rect(0, y, (float)Display::width, hmax).Inside(Input::mousePos)) {
					y0 -= Input::mouseScroll * 20;
					y0 = std::min(y0, nh * 17.f - hmax);
					y0 = std::max(y0, 0.f);
				}
				int a = 0;
				float yp = y - y0;
				while (yp < y - 18.f) {
					yp += 17.f;
					a += nw;
				}
				bool sw = false;
				for (; a < ts; a++) {
					if (dim > 1 && !((a+1) % szs[0]))
						sw = !sw;
					if (!ix) {
						UI::Label(5, yp, 12, std::to_string(a) + ":", white(0.9f));
					}
					std::string vl = "";
					switch (type) {
					case 's':
						vl = std::to_string(((short*)data)[a]);
						break;
					case 'i':
						vl = std::to_string(((int*)data)[a]);
						break;
					case 'd':
						vl = std::to_string(((double*)data)[a]);
						break;
					default:
						OHNO("ArrayView", "Unexpected type: " + std::string(1, type) + "!");
						return;
					}
					const float x = 55.f + ix * dw;
					UI::Quad(x, yp, dw-1.f, 16, white(0.9f, sw? 0.2f : 0.3f));
					UI::Label(x + 2, yp, 12, vl, white(0.9f));
					if (++ix == nw) {
						ix = 0;
						yp += 17;
						if (yp > Display::height - 17.f) break;
					}
				}
				Engine::EndStencil();
			}
		}
	}
}