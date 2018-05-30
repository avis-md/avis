#include "popups.h"
#include "vis/pargraphics.h"

POPUP_TYPE Popups::type = POPUP_TYPE::NONE;
Vec2 Popups::pos = Vec2();
void* Popups::data = 0;

void Popups::Draw() {
    if (type == POPUP_TYPE::NONE) return;
    UI::IncLayer();

    switch (type) {
        case POPUP_TYPE::DRAWMODE:
            ParGraphics::DrawPopupDM();
    }
}