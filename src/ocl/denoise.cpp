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

#include "denoise.h"

oidn::DeviceRef Denoise::device;
oidn::FilterRef Denoise::filter;

void Denoise::Init() {
	Debug::Message("Denoise", "Initializing denoiser...");
	device = oidn::newDevice();
	device.commit();
	filter = device.newFilter("RT");
	filter.set("hdr", true);
}

void Denoise::Apply(float* img, int w, int h, float* res) {
	if (!filter) Init();

	filter.setImage("color", img, oidn::Format::Float3, w, h);
	filter.setImage("output", res, oidn::Format::Float3, w, h);
	filter.commit();
	filter.execute();
}