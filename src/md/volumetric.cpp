#include "volumetric.h"

std::vector<Volumetric::DataFrame> Volumetric::frames;
Volumetric::DataFrame* Volumetric::currentFrame = nullptr;

void Volumetric::Resize(uint16_t szs[3]) {
    if (!szs[0] || !szs[1] || !szs[2]) return;
    frames.push_back(DataFrame(szs[0], szs[1], szs[2]));
    currentFrame = &frames[0];
}

void Volumetric::Clear() {
    frames.clear();
    currentFrame = nullptr;
}