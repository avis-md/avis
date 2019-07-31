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

namespace config {
	const char* env[] = {
#ifdef PLATFORM_WIN
		"PYENV", "Python Root", "C:/Python37",
		"Directory of Python installation. Directory should contain the libs/ folder.",

		"MINGW", "MinGW Bin", "C:/MinGW/bin",
		"Directory of MinGW binaries. Directory should contain compiler executables.",

		"VCBAT", "Vcvars path", "C:/Program Files/Microsoft Visual Studio/VC/bin/vcvars64.bat",
		"Path of the vcvars bat file of visual studio.",
#endif

		"GPP", "g++ name", "g++",
		"Name of the g++ compiler. Possible names: g++, g++-mp, g++-8",
		
		nullptr
	};
}