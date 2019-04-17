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