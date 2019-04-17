#include <iostream>
#include "common_math.h"

//in pcnt 3
double* coords = 0;
//in mcnt 4
int* tetras = 0;
//in
double size = 0;

//out mcnt4 3
double* poss = 0;
//out mcnt4 3
double* nrms = 0;

//var
int pcnt = 0;
//var
int mcnt = 0;
//var
int mcnt4 = 0;

#define NARR(tp, nm, sz) if (nm) delete[](nm); nm = new tp[sz]{};

//entry
void Do() {
    mcnt4 = mcnt * 12;
    NARR(double, poss, mcnt4 * 3);
    NARR(double, nrms, mcnt4 * 3);
    
    const int trs[12] = {
        0, 1, 2,  0, 1, 3,
        0, 2, 3,  1, 2, 3
    };
    
    for (int a = 0; a < mcnt; a++) { //for each tetra
        double cen[3] = {};
        double vts[12] = {};
        for (int b = 0; b < 4; b++) { //for each vert
            memcpy(vts + b * 3, coords + tetras[a * 4 + b] * 3, 3 * sizeof(double));
            add(cen, vts + b * 3, cen);
        }
        mul(cen, 0.25, cen);
        
        double nms[12] = {};
        
        for (int b = 0; b < 4; b++) {
            double* v1 = &vts[trs[b * 3] * 3];
            double* v2 = &vts[trs[b * 3 + 1] * 3];
            double* v3 = &vts[trs[b * 3 + 2] * 3];
            double tmp[3], t1[3], t2[3];
            vec(v2, v1, t1);
            vec(v3, v1, t2);
            double* n = &nms[b*3];
            cross(t1, t2, n);
            vec(v1, cen, tmp);
            if (dot(n, tmp) < 0)
                mul(n, -1, n);
            mul(n, 1/len(n), n);
        }
        
        for (int b = 0; b < 12; b++) {
            double tmp[3] = {};
            double* v = &vts[trs[b] * 3];
            vec(v, cen, tmp);
            mul(tmp, size, tmp);
            add(cen, tmp, poss + (a * 12 + b) * 3);
            
            memcpy(nrms + (a * 12 + b) * 3, &nms[(b / 3) * 3], 3 * sizeof(double));
        }
    }
}