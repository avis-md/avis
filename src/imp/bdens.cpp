#include "bdens.h"
#include <fstream>

#define CP_BLOCK_SZ 65535

#define SETERR(msg) memcpy(info->error, msg, sizeof(msg))

#define BDens_COPY(tar, prgs, T)\
    for (int a = 0; a < n; a += CP_BLOCK_SZ) {\
        prgs = ((float)a) / n;\
        strm.read((char*)&tar[a], T * std::min(n - a, CP_BLOCK_SZ));\
    }

template <typename T>
void BDens_Read(std::ifstream& strm, int n, double* data, float* prgs) {
    std::vector<T> _data(CP_BLOCK_SZ);
    for (int a = 0; a < n; a += CP_BLOCK_SZ) {
        *prgs = ((float)a) / n;
        const int m = std::min(n - a, CP_BLOCK_SZ);
        strm.read((char*)_data.data(), sizeof(T) * m);
        for (int b = 0; b < m; b++) {
            data[a + b] = (double)_data[b];
        }
    }
}

bool BDens::Read(ParInfo* info) {
    std::ifstream strm(PATH(info->path), std::ios::binary);
    if (!strm) return false;
    strm.read((char*)info->densityNum, sizeof(uint16_t) * 3);
    if (strm.eof()) return false;
    const auto n = info->densityNum[0]*info->densityNum[1]*info->densityNum[2];
    info->density = new double[n];
    char type;
    strm.read(&type, 1);
    switch (type) {
    case 'D':
        BDens_COPY(info->density, info->progress, sizeof(double));
        break;
    case 'F':
        BDens_Read<float>(strm, n, info->density, &info->progress);
        break;
    case 'S':
        BDens_Read<int16_t>(strm, n, info->density, &info->progress);
        break;
    case 'I':
        BDens_Read<int32_t>(strm, n, info->density, &info->progress);
        break;
    case 'L':
        BDens_Read<int64_t>(strm, n, info->density, &info->progress);
        break;
    default:
        SETERR(("Unknown type \"" + std::string(&type, 1) + "\"!").c_str());
        break;
    }
    if (strm.bad()) {
        delete[] info->density;
        return false;
    }
    info->bounds[0] = -info->densityNum[0] * 0.5;
    info->bounds[1] = info->densityNum[0] * 0.5;
    info->bounds[2] = -info->densityNum[1] * 0.5;
    info->bounds[3] = info->densityNum[1] * 0.5;
    info->bounds[4] = -info->densityNum[2] * 0.5;
    info->bounds[5] = info->densityNum[2] * 0.5;
    return true;
}