#include <iostream>
#include <fstream>
#include <random>
#include <vector>

VARIN float threshold = 0;
VARIN float inc = 0;
VARIN int foc = 0;

VECSZ(tcnt, pcnt)
VARIN float* icols = 0;

VECSZ(tcnt, pcnt)
VAROUT float* cols = 0;

//VECSZ(fcnt)
//VAROUT int* focs = 0;

VECVAR int tcnt = 2;
VECVAR int pcnt = 2;

//VECVAR int fcnt = 0;

ENTRY void Execute() {
	if (!cols) cols = new float[tcnt * pcnt] {};
	
	for (int i = 0; i < tcnt * pcnt; i++) {
		cols[i] = (icols[i] > threshold)? 0.5f : 0;
	}
	for (int i = 1; i < tcnt; i++) {
		for (int j = 0; j < pcnt; j++) {
			if (cols[i * pcnt + j] > 0.3f && cols[(i-1) * pcnt + j] > 0.3f) {
				cols[i * pcnt + j] = cols[(i-1) * pcnt + j] + inc;
			}
		}
	}
	
/*	for (int j = 0; j < pcnt; j++) {
		if (cols[(tcnt-1) * pcnt + j] > 0.3f && ) {
			
		}
	} */
}