#include "selection.h"
#include "md/Particles.h"
#include "md/ParMenu.h"
#include "ui/icons.h"
#include "ui/localizer.h"

bool Selection::dirty = false;
size_t Selection::count;
std::vector<uint> Selection::atoms;
std::vector<double> Selection::lengths, Selection::angles, Selection::torsions;

bool Selection::expL = true, Selection::expA = true, Selection::expT = true;
int Selection::_dirty = 0;

void Selection::Clear() {
    atoms.clear();
    lengths.clear();
    angles.clear();
    torsions.clear();
}

void Selection::Recalc() {
    dirty = true;
    _dirty = 0;
}

void Selection::Calc1() {
    if (!dirty || _dirty) return;
    if (count > 1) {
        auto& p = Particles::poss[atoms[0]];
        auto& q = Particles::poss[atoms[1]];
        lengths.resize(1, glm::length(q - p));
        if (count > 2) {
            auto& r = Particles::poss[atoms[2]];
            angles.resize(1, std::acos(glm::dot(glm::normalize(p-q), glm::normalize(r-q))));
            if (count > 3) {
                auto& s = Particles::poss[atoms[3]];
                auto x1 = glm::normalize(glm::cross(p-q, r-q));
                auto x2 = glm::normalize(glm::cross(q-r, s-r));
                torsions.resize(1, std::acos(glm::dot(x1, x2)));
            }
        }
    }
    dirty = false;
    _dirty = 0xff;
}

void Selection::CalcLen() {
    if (!(_dirty & 1)) return;
    if (count > 1) {
        lengths.resize(count-1);
#pragma omp parallel for
        for (int a = 0; a < (int)count-1; a++) {
            auto& p = Particles::poss[atoms[a]];
            auto& q = Particles::poss[atoms[a+1]];
            lengths[a] = glm::length(q - p);
        }
    }
    _dirty &= ~1;
}

void Selection::CalcAng() {
    if (!(_dirty & 2)) return;
    if (count > 2) {
        angles.resize(count-2);
#pragma omp parallel for
        for (int a = 0; a < (int)count-2; a++) {
            auto& p = Particles::poss[atoms[a]];
            auto& q = Particles::poss[atoms[a+1]];
            auto& r = Particles::poss[atoms[a+2]];
            angles[a] = std::acos(glm::dot(glm::normalize(p-q), glm::normalize(r-q)));
        }
    }
    _dirty &= ~2;
}

void Selection::CalcTor() {
    if (!(_dirty & 4)) return;
    if (count > 3) {
        torsions.resize(count-3);
#pragma omp parallel for
        for (int a = 0; a < (int)count-3; a++) {
            auto& p = Particles::poss[atoms[a]];
            auto& q = Particles::poss[atoms[a+1]];
            auto& r = Particles::poss[atoms[a+2]];
            auto& s = Particles::poss[atoms[a+3]];
            auto x1 = glm::normalize(glm::cross(p-q, r-q));
            auto x2 = glm::normalize(glm::cross(q-r, s-r));
            torsions[a] = std::acos(glm::dot(x1, x2));
        }
    }
    _dirty &= ~4;
}

void Selection::DrawMenu() {
    float off = 20;
    const float ep = ParMenu::expandPos;
    count = atoms.size();
    UI::Label(ep - 148, off, 12, "Information", white());
    Calc1();
    off += 30;
    if (count > 1) {
        if (count > 2) {
            if (Engine::Button(ep - 148, off, 16, 16, expL ? Icons::expand : Icons::collapse) == MOUSE_RELEASE) {
                expL = !expL;
            }
            UI::Label(ep - 130, off, 12, "Distance", white());
            off += 17;
            if (expL) {
                CalcLen();
                for (size_t a = 0; a < count-1; a++) {
                    UI::Label(ep - 146, off, 12, std::to_string(a) + "~" + std::to_string(a+1) + ": " + std::to_string(lengths[a]) + " nm", white());
                    off += 17;
                }
            }

            off += 17;

            if (count > 3) {
                if (Engine::Button(ep - 148, off, 16, 16, expL ? Icons::expand : Icons::collapse) == MOUSE_RELEASE) {
                    expA = !expA;
                }
                UI::Label(ep - 130, off, 12, "Angle", white());
                off += 17;
                if (expA) {
                    CalcAng();
                    for (size_t a = 0; a < count-2; a++) {
                        UI::Label(ep - 146, off, 12, std::to_string(a) + "~" + std::to_string(a+2) + ": " + std::to_string(angles[a]) + " rad", white());
                        off += 17;
                    }
                }

                off += 17;
                
                if (count > 4) {
                    if (Engine::Button(ep - 148, off, 16, 16, expL ? Icons::expand : Icons::collapse) == MOUSE_RELEASE) {
                        expA = !expA;
                    }
                    UI::Label(ep - 130, off, 12, "Torsion", white());
                    off += 17;
                    if (expA) {
                        CalcTor();
                        for (size_t a = 0; a < count-2; a++) {
                            UI::Label(ep - 146, off, 12, std::to_string(a) + "~" + std::to_string(a+3) + ": " + std::to_string(angles[a]) + " rad", white());
                            off += 17;
                        }
                    }
                }
                else {
                    UI::Label(ep - 147, off, 12, "Torsion: " + std::to_string(torsions[0]) + " rad", white());
                }
            }
            else {
                UI::Label(ep - 147, off, 12, "Angle: " + std::to_string(angles[0]) + " rad", white());
            }
        }
        else {
            UI::Label(ep - 147, off, 12, "Distance: " + std::to_string(lengths[0]) + " nm", white());
        }
    }
}