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