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
				Engine::BeginStencil(0, y, Display::width, Display::height - y - 18.f);
				int dw = 71;
				int nw = (int)std::floor((Display::width - 10) / dw);
				dw = (Display::width - 10) / nw;
				int ix = 0;
				bool sw = false;
				for (int a = 0; a < ts; a++) {
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
					const int x = 5 + ix * dw;
					UI::Quad(x, y, dw-1, 16, white(0.9f, sw? 0.2f : 0.3f));
					UI::Label(x + 2, y, 12, vl, white(0.9f));
					if (++ix == nw) {
						ix = 0;
						y += 17;
						if (y > Display::height - 17.f) break;
					}
					if (dim > 1 && !((a+1) % szs[0]))
						sw = !sw;
				}
				Engine::EndStencil();
			}
		}
	}
}