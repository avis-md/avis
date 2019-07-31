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