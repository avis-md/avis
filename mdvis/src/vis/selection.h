#pragma once
#include "Engine.h"

class Selection {
public:
    static bool dirty;
    static size_t count;
    static std::vector<uint> atoms;
    static std::vector<double> lengths, angles, torsions;

    static void Clear(), Recalc(), Calc1(), CalcLen(), CalcAng(), CalcTor();
    static void DrawMenu();

private:
    static bool expL, expA, expT;
    static int _dirty;
};