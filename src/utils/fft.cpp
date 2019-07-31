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

#include "fft.h"

std::vector<std::complex<float>> FFT::Evaluate(const std::vector<float>& vals, FFT_WINDOW window) {
	const auto& sz = vals.size();
	assert(isGoodLength(sz));
	std::vector<std::complex<float>> res(sz);
	for (uint i = 0; i < sz; ++i) {
		res[i] = applyWindow(vals[i], i * 1.f / (sz - 1));
	}
	doFft(&res[0], sz);
	return res;
}

bool FFT::isGoodLength(uint v) {
	for (uint a = 1; a <= 31; ++a) {
		if (v == (1 << a)) return true;
	}
	return false;
}

float FFT::applyWindow(float val, float pos) {
	return val;
}

void FFT::doFft(std::complex<float>* v, uint c) {
	if (c >= 2) {
		const uint hc = c / 2;
		separate(v, hc);
		doFft(v, hc);
		doFft(v + hc, hc);
		for (uint i = 0; i < hc; ++i) {
			auto e = v[i];
			auto o = v[i + hc];
			auto w = exp(std::complex<float>(0, -2.f*PI * i / c));
			v[i] = e + w * o;
			v[i + hc] = e - w * o;
		}
	}
}

void FFT::separate(std::complex<float>* v, uint hc) {
	std::vector<std::complex<float>> t(hc);
	for (uint i = 0; i < hc; ++i) {
		t[i] = v[i * 2 + 1];
	}
	for (uint i = 0; i < hc; ++i) {
		v[i] = v[i * 2];
	}
	for (uint i = 0; i < hc; ++i) {
		v[i + hc] = t[i];
	}
}