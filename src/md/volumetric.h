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
#include "Engine.h"
#include "vis/system.h"

class Volumetric {
public:
    struct DataFrame {
        DataFrame() : nx(0), ny(0), nz(0) {}
        DataFrame(int x, int y, int z)
         : nx(x), ny(y), nz(z), data(x*y*z, 0) {}
        int nx, ny, nz;
        std::vector<double> data;
    };

    static std::vector<DataFrame> frames;

    static DataFrame* currentFrame;

    static void Resize(uint16_t szs[3]);
    static void Clear();
};