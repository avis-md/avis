#pragma once

/*
Format: signature, name, type[B / I / F / S], {args},
		description,

  signature must start with [S(ystem) / A(nalysis) / V(isualization)]

Args:
	Bool: value[0 / 1]
	Int: value, min, max, use slider
	Float: value, min, max, use slider
	String: value
	Color: value.r, value.g, value.b, value.a

  if min == max then no range is imposed
*/

namespace config {
	const char* prefs[] = {
		"SDPI", "DPI Scale Factor", "F", "1", "0.2", "5", "N",
		"Scale Factor of the User Interface. Large numbers make the icons larger and vice versa.",

		"SHQUI", "High Quality UI", "B", "1",
		"Blurring of user interface background",

		"SUIBL", "UI Blur Amount", "F", "10", "0.1", "10", "Y",
		"Blur size of user interface background",

		"SOPUI", "UI Opacity", "F", "0.7", "0", "1", "Y",
		"Non-see-through-ness of the user interface",

		"SBKCL", "UI Background Color", "C", "0.1", "0.1", "0.1", "1",
		"Tint color of the user interface",

		"SACCL", "UI Accent Color", "C", "1", "0.75", "0", "1",
		"Font accent color of the user interface",

#ifdef PLATFORM_WIN
		"AMSVC", "Use MSVC", "B", "0",
		"Use the Visual Studio compiler instead of MinGW",
#endif

		"ACL", "CL Flags", "S", "",
		"Additional flags to pass to the compiler. Include libraries here if necessary.",

		"ALNK", "LNK Flags", "S", "",
		"Additional flags to pass to the linker. Include libraries here if necessary.",

		"AOMP", "Enable OpenMP", "B", "0",
		"Enables OpenMP when compiling. OpenMP must be supported by the compiler.",

		"AOMPL", "Link OpenMP Libs", "B", "0",
		"Links against the OpenMP libraries. OpenMP libraries must be separately installed.",

		"VSS", "Scroll Speed", "F", "1", "0.2", "5", "Y",
		"Mouse ScrollWheel Sensitivity",

		"VAX", "Show Axes", "B", "1",
		"Show axes in bottom left corner of 3D view",

		"VAS", "Axes size", "F", "15", "10", "30", "Y",
		" ",

		"VCX", "X axis color", "C", "1", "0", "0", "1",
		" ",

		"VCY", "Y axis color", "C", "0", "0.7", "0", "1",
		" ",

		"VCZ", "Z axis color", "C", "0", "0", "1", "1",
		" ",


		nullptr
	};
}