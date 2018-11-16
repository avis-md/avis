#pragma once
#include "Engine.h"

enum FFT_WINDOW : byte {
	FFT_WINDOW_RECTANGLE,
	FFT_WINDOW_HAMMING,
	FFT_WINDOW_BLACKMAN
};

/*! Cooley-Tukey FFT
[av] */
class FFT {
public:
	static std::vector<std::complex<float>> Evaluate(const std::vector<float>& values, FFT_WINDOW window);

private:
	static bool isGoodLength(uint a);
	static float applyWindow(float val, float pos);
	static void doFft(std::complex<float>* v, uint c);
	static void separate(std::complex<float>* v, uint c);
};