#include <iostream>
#include <cmath>

//in 6
double* bbox = 0;
//in
int count = 0;

//in parcnt 3
double* positions = 0;

//in 1
short* types = 0;

//var
int parcnt = 0;

//out count 3
double* density = 0;

//entry
void Do () {
    double zmin = bbox[4];
    double zmax = bbox[5];
    double vol = (bbox[1]-bbox[0])*(bbox[3]-bbox[2])*(zmax-zmin);
    vol /= count;
    double dd = 1/vol;
    if (density) delete[](density);
    density = new double[count*3]{};
    int skip = 0;
    for (int a = 0; a < parcnt; a++) {
        double z = positions[a*3 + 2];
        z = (z - zmin)/(zmax-zmin);
        auto i = (int)floor(z*count);
        if (i >= 0 && i < count) {
            short tp = types[a];
            if (tp == *(short*)"O")
                density[i*3] += dd;
            else if (tp == *(short*)"H")
                density[i*3+1] += dd;
            else if (tp == *(short*)"C")
                density[i*3+2] += dd;
            else skip++;
        }
    }
    if (skip>0)
        std::cout << "skipped " << skip << " atoms" << std::endl;
}