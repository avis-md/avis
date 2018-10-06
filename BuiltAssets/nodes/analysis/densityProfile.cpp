#include <iostream>
#include <cmath>
#include <vector>
#include <unordered_map>
#include <cstring>

//in 6
double* bbox = 0;
//in enum X Y Z
int axis = 0;
//in
int count = 0;

//in parcnt 3
double* positions = 0;

//in 1
short* types = 0;

//var
int parcnt = 0;

//out count typecnt
double* density = 0;

//var
int typecnt = 0;

//entry
void Do () {
    if (count <= 0) {
        std::cerr << "count must be positive!" << std::endl;
        return;
    }
    double zmin = bbox[axis*2];
    double zmax = bbox[axis*2+1];
    double vol = (bbox[1]-bbox[0])*(bbox[3]-bbox[2])*(bbox[5]-bbox[4]);
    vol /= count;
    double dd = 1/vol;
    if (density) delete[](density);
    
    std::unordered_map<short, std::vector<double>> dens;
    
    for (int a = 0; a < parcnt; a++) {
        double z = positions[a*3 + axis];
        z = (z - zmin)/(zmax-zmin);
        auto i = (int)floor(z*count);
        if (i >= 0 && i < count) {
            short tp = types[a];
            dens[tp].resize(count);
            dens[tp][i] += dd;
        }
    }
    typecnt = (int)dens.size();
    density = new double[count * typecnt];
    int a = 0;
    for (auto& d : dens) {
        for (int x = 0; x < count; x++)
            density[x*typecnt + a] = d.second[x];
        a++;
    }
}