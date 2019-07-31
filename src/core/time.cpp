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

#include "Engine.h"
#ifndef PLATFORM_WIN
#include <sys/time.h>
#endif

long long Time::startMillis = 0;
long long Time::millis = 0;
float Time::time = 0;
float Time::delta = 0;

long long milliseconds() {
#ifdef PLATFORM_WIN
	static LARGE_INTEGER s_frequency;
	static BOOL s_use_qpc = QueryPerformanceFrequency(&s_frequency);
	if (s_use_qpc) {
		LARGE_INTEGER now;
		QueryPerformanceCounter(&now);
		return (1000LL * now.QuadPart) / s_frequency.QuadPart;
	}
	else {
		return GetTickCount64();
	}
#else
	struct timeval time;
	gettimeofday(&time, 0);
	return time.tv_sec * 1000 + time.tv_usec / 1000;
#endif
}

void Time::Update() {
	long long millis = milliseconds();
	Time::delta = (millis - Time::millis)*0.001f;
	Time::time = (millis - Time::startMillis)*0.001f;
	Time::millis = millis;
}