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