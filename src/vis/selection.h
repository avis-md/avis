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

class Selection {
public:
    static bool dirty;
    static size_t count;
    static std::vector<uint> atoms;
    static std::vector<Vec2> spos;
    static std::vector<double> lengths, angles, torsions;

    static void Clear(), Recalc(), Calc1();
    static void CalcSpos(), CalcLen(), CalcAng(), CalcTor();
    static void DrawMenu();

private:
    static bool expL, expA, expT;
    static int _dirty;
};