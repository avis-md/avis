#include <iostream>

VARIN float in = 0;

VAROUT float out = 0;

VECSZ(VEC_MAX_PAR)
VARIN float* vec1 = 0;

VECVAR int VEC_MAX_PAR = 2;

ENTRY void Execute() {
	std::cout << "size of VEC_MAX_PAR = " << VEC_MAX_PAR << std::endl;
	for (int a = 0; a < VEC_MAX_PAR; a++)
		out += vec1[a];
	out *= in;
	std::cout << "sum = " << out << "!" << std::endl;
	//vec2 = new float[]{ 1.5f, 2.5f, 3.5f, 2.5f, 3.5f, 4.5f };
}